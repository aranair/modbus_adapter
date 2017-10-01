#include <stdio.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus.h>
#include <time.h>

#define PLC_SERVER_ID 1
#define KEP_SERVER_ID 2

#define FREQ_ADDR_OFFSET 0

enum {
  TCP,
  RTU
};

void set_inverter_speed(modbus_t *ctx, uint16_t addr_offset, uint16_t hertz)
{
  uint16_t data[1] = { hertz * 100 };
  int res = modbus_write_registers(ctx, addr_offset, 1, data);
  if (res != 1) {
    fprintf(stderr, "Failed to write register: %s\n", modbus_strerror(errno));
  }
}

void print_register_results(int resp_n, int n, uint16_t* reg)
{
  if (resp_n != n) {
    fprintf(stderr, "Error: %s\n", modbus_strerror(errno));
  } else {
    for (int i = 0; i < n; i++) {
      printf("%d: %d\n", i, reg[i]);
    }
    printf("\n");
  }
}

int main(int argc, char*argv[])
{
  modbus_t *ctx;
  int      res;
  int      use_backend;
  uint16_t data[2];

  srand(time(NULL));

  if (argc > 1) {
    if (strcmp(argv[1], "tcp") == 0) {
      use_backend = TCP;
    } else if (strcmp(argv[1], "rtu") == 0) {
      use_backend = RTU;
    } else {
      return -1;
    }
  } else {
    use_backend = TCP;
  }

  if (use_backend == TCP) {
    ctx = modbus_new_tcp("127.0.0.1", 1502);
  } else {
    ctx = modbus_new_rtu("/dev/ttys029", 115200, 'N', 8, 1);
    modbus_set_slave(ctx, PLC_SERVER_ID);
  }

  /* modbus_set_debug(ctx, TRUE); */
  if (!ctx) {
    fprintf(stderr, "Failed to create the context: %s\n", modbus_strerror(errno));
    exit(1);
  }

  if (modbus_connect(ctx) == -1) {
    fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
    modbus_free(ctx);
    return -1;
  }

  for (;;) {
    /* Read registers from inverter */
    res = modbus_read_registers(ctx, FREQ_ADDR_OFFSET, 2, data);
    print_register_results(res, 2, data);

    /* Write values from inverter to another client (?) */
    /* Could also broadcast instead if sending directly doesn't work (?) */
    modbus_set_slave(ctx, KEP_SERVER_ID);
    res = modbus_write_registers(ctx, FREQ_ADDR_OFFSET, 2, data);
    if (res != 1) {
      fprintf(stderr, "Failed to write registers: %s\n", modbus_strerror(errno));
    }

    /* Update speed randomly values and write it back to inverter */
    set_inverter_speed(ctx, FREQ_ADDR_OFFSET, rand() % 25 + 1);

    res = modbus_read_registers(ctx, FREQ_ADDR_OFFSET, 2, data);
    print_register_results(res, 2, data);

    sleep(3);
  } /* end for */

  printf("Exit the loop: %s\n", modbus_strerror(errno));
  modbus_close(ctx);
  modbus_free(ctx);
}
