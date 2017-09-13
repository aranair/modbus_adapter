#include <stdio.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus.h>
#include <stdbool.h>
#include "utility.h"

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
/*     fprintf(stderr, "Read estop coil error: %s\n", modbus_strerror(errno)); */
/*   else */
/*     printf("Real Estop (0x9): %d\n", coil_bit[0]); */
/* } */

int main(int argc, char*argv[])
{
  modbus_t *ctx;
  modbus_t *tcp_ctx;
  int mIterations = 0;

  tcp_ctx = modbus_new_tcp("10.0.0.164", 1502);
  modbus_set_slave(tcp_ctx, KEP_SERVER_ID);

  ctx = modbus_new_rtu("/dev/ttys020", 115200, 'N', 8, 1);
  modbus_set_slave(ctx, PLC_SERVER_ID);

  if (!ctx || !tcp_ctx) {
    fprintf(stderr, "Failed to connect context: %s\n", modbus_strerror(errno));
    exit(1);
  }

  if (modbus_connect(ctx) == -1) {
    fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
    modbus_free(ctx);
    exit(1);
  }

  if (modbus_connect(tcp_ctx) == -1) {
    fprintf(stderr, "Connection failed to tcp_ctx: %s\n", modbus_strerror(errno));
    modbus_free(tcp_ctx);
    exit(1);
  }

  set_speed(ctx, WRITE_FREQ_ADDR_OFFSET, 15 * 100);
  printf("Done\n");

  set_coil(ctx, WRITE_ESTOP_COIL_ADDR_OFFSET, true);
  printf("Done\n");

  /* sleep(2); */
  /* set_speed(ctx, WRITE_FREQ_ADDR_OFFSET, 0); */
  /* set_coil(ctx, WRITE_ESTOP_COIL_ADDR_OFFSET, false); */

  int16_t real_freq = read_register(ctx, 0);
  printf("real_freq: %i\n", real_freq);

  int8_t estop_coil = read_coil(ctx, 0);
  printf("coil: %i\n", estop_coil);

  /* Relay values to Kepware via TCP */
  set_coil(tcp_ctx, WRITE_ESTOP_COIL_ADDR_OFFSET, estop_coil);
  printf("Done\n");
  set_speed(tcp_ctx, WRITE_FREQ_ADDR_OFFSET, real_freq);
  printf("Done\n");


  printf("Exit the loop.\n");
  modbus_close(ctx);
  modbus_free(ctx);
}
