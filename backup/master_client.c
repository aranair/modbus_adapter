#include <stdio.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus.h>
#include <stdbool.h>

#define PLC_SERVER_ID 1
#define KEP_SERVER_ID 2

#define WRITE_FREQ_ADDR_OFFSET 0
#define WRITE_ESTOP_COIL_ADDR_OFFSET 0

#define READ_FREQ_ADDR_OFFSET  0x1001
#define READ_ESTOP_COIL_ADDR_OFFSET 0x9

enum {
  TCP,
  RTU
};

/* void test_methods(modbus_t *ctx) */
/* { */
/*   uint16_t data[1]; */
/*   uint8_t  coil_bit[1]; */

/*   // Read register- real frequency 0x1001 */
/*   if (modbus_read_registers(ctx, READ_FREQ_ADDR_OFFSET, 1, data) != 1) */
/*     fprintf(stderr, "Read real freq register error: %s\n", modbus_strerror(errno)); */
/*   else */
/*     printf("Real Frequency (0x1001): %d\n", data[0]); */

/*   // Read estop coil */
/*   if (modbus_read_bits(ctx, READ_ESTOP_COIL_ADDR_OFFSET, 1, coil_bit) != 1) */
/*     fprintf(stderr, "Read register error: %s\n", modbus_strerror(errno)); */
/*   else */
/*     printf("Real Estop (0x9): %d\n", coil_bit[0]); */
/* } */

void set_coil(modbus_t *ctx, uint16_t addr_offset, bool setting)
{
  printf("Setting coil to %d\n", setting);
  if (modbus_write_bit(ctx, addr_offset, setting ? 1 : 0) != 1) {
    fprintf(stderr, "Failed to write to coil: %s\n", modbus_strerror(errno));
  }
}

void set_speed(modbus_t *ctx, uint16_t addr_offset, uint16_t hertz)
{
  uint16_t data[2] = { 0, hertz * 100 };
  printf("Setting speed to %d Hz\n", hertz);
  if (modbus_write_registers(ctx, addr_offset, 2, data) != 2) {
    fprintf(stderr, "Failed to write register: %s\n", modbus_strerror(errno));
  }
}

int8_t read_coil(modbus_t *ctx, uint16_t addr_offset)
{
  uint8_t coil_bit[1];
  if (modbus_read_bits(ctx, addr_offset, 1, coil_bit) == 1) {
    return coil_bit[0];
  } else {
    fprintf(stderr, "Read coil bit error: %s\n", modbus_strerror(errno));
    return -1;
  }
}

int16_t read_register(modbus_t *ctx, uint16_t addr_offset)
{
  uint16_t data[1];
  if (modbus_read_registers(ctx, READ_FREQ_ADDR_OFFSET, 1, data) == 1) {
    return data[0];
  } else {
    fprintf(stderr, "Read register error: %s\n", modbus_strerror(errno));
    return -1;
  }
}

int main(int argc, char*argv[])
{
  modbus_t *ctx;
  int use_backend = TCP;
  int mIterations = 0;

  if (argc > 1 && strcmp(argv[1], "rtu") == 0) use_backend = RTU;

  /* Initialize connection */
  if (use_backend == TCP) {
    ctx = modbus_new_tcp("10.0.0.164", 1502);
  } else {
    /* ctx = modbus_new_rtu("/dev/ttys029", 115200, 'N', 8, 1); */
    ctx = modbus_new_rtu("/dev/ttyUSB1", 115200, 'N', 8, 1);
    modbus_set_slave(ctx, PLC_SERVER_ID);
  }

  if (!ctx) {
    fprintf(stderr, "Failed to create the context: %s\n", modbus_strerror(errno));
    exit(1);
  }

  if (modbus_connect(ctx) == -1) {
    fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
    modbus_free(ctx);
    exit(1);
  }

  /* modbus_set_debug(ctx, TRUE); */
  /* test_methods(ctx); */

  for (;;) {
    /* Rotate speed */
    switch (mIterations % 3500) {
      case 500:
        set_speed(ctx, WRITE_FREQ_ADDR_OFFSET, 15);
        set_coil(ctx, WRITE_ESTOP_COIL_ADDR_OFFSET, true);
        break;

      case 1000:
        set_speed(ctx, WRITE_FREQ_ADDR_OFFSET, 25);
        break;

      case 2000:
        set_speed(ctx, WRITE_FREQ_ADDR_OFFSET, 10);
        break;

      case 3000:
        set_coil(ctx, WRITE_ESTOP_COIL_ADDR_OFFSET, false);
        set_speed(ctx, WRITE_FREQ_ADDR_OFFSET, 0);
        break;

      case 3400:
        mIterations = 0;
        break;

      default:
        break;
    }

    mIterations++;

    // Read the real values
    int16_t real_freq = read_register(ctx, READ_FREQ_ADDR_OFFSET);
    /* printf("real freq: %i \n", real_freq); */

    int8_t estop_coil = read_coil(ctx, READ_ESTOP_COIL_ADDR_OFFSET);
    /* printf("real freq: %i \n", estop_coil); */

    // Rest a little before retrying
    if (real_freq == -1 || estop_coil == -1) {
      sleep(1);
      continue;
    }

    /* Write coil value to Kepware */
    /* set_coil(tcp_ctx, WRITE_ESTOP_COIL_ADDR_OFFSET, true); */

    /* Write registers value to Kepware */
    /* set_speed(tcp_ctx, WRITE_FREQ_ADDR_OFFSET, real_freq / 100); */
  }

  printf("Exit the loop.\n");
  modbus_close(ctx);
  modbus_free(ctx);
}
