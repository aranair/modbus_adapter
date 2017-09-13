#include <modbus.h>
#include <time.h>

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
  /* uint16_t data[2] = { 0, hertz * 100 }; */
  uint16_t data[1] = { hertz * 100 };
  printf("Setting speed to %d Hz\n", hertz);
  /* if (modbus_write_registers(ctx, addr_offset, 2, data) != 2) { */
  if (modbus_write_registers(ctx, addr_offset, 1, data) != 1) {
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
  if (modbus_read_registers(ctx, addr_offset, 1, data) == 1) {
    return data[0];
  } else {
    fprintf(stderr, "Read register error: %s\n", modbus_strerror(errno));
    return -1;
  }
}

// This is for the windows nanosleep!
#ifdef WIN32
#include <windows.h>
#elif _POSIX_C_SOURCE >= 199309L
#include <time.h>   // for nanosleep
#else
#include <unistd.h> // for usleep
#endif
void sleep_ms(int milliseconds) // cross-platform sleep function
{
#ifdef WIN32
    Sleep(milliseconds);
#elif _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
#else
    usleep(milliseconds * 1000);
#endif
}
