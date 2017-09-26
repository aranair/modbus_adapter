#include <modbus.h>
#include <time.h>

void set_coil(modbus_t *ctx, uint16_t addr_offset, bool setting)
{
  printf("Setting coil to %d\n", setting);
  if (modbus_write_bit(ctx, addr_offset, setting ? 1 : 0) != 1) {
    fprintf(stderr, "Failed to write to coil: %s\n", modbus_strerror(errno));
  }
}

void set_kep_req_speed(modbus_t *ctx, uint16_t addr_offset, uint16_t hertz)
{
	uint16_t data[1] = { hertz };
	if (modbus_write_registers(ctx, addr_offset, 1, data) != 1) {
		fprintf(stderr, "Failed to write register: %s\n", modbus_strerror(errno));
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

int read_coil(modbus_t *ctx, uint16_t addr_offset, uint8_t *ret)
{
  uint8_t coil_bit[1];
  if (modbus_read_bits(ctx, addr_offset, 1, coil_bit) == 1) {
    *ret = coil_bit[0];
    return 1;
  } else {
    fprintf(stderr, "Read coil bit error: %s\n", modbus_strerror(errno));
    return -1;
  }
}

int read_register(modbus_t *ctx, uint16_t addr_offset, uint16_t *ret)
{
  uint16_t data[1];
  if (modbus_read_registers(ctx, addr_offset, 1, data) == 1) {
    *ret = data[0];
    return 1;
  } else {
    fprintf(stderr, "Read register error: %s\n", modbus_strerror(errno));
    return -1;
  }
}

/* Returns a pointer to the device that matches the name in the config. */
struct ModbusDevice *get_device(struct ModbusConfig *config, char *name) {
  for (int i = 0; i < config->device_count; i++) {
    if (strcmp(config->devices[i].name, name) == 0) {
      return &config->devices[i];
    }
  }
  return NULL;
}

/* Returns a pointer to the data object that matches the name, scoped to device */
struct ModbusData * get_data(struct ModbusDevice *device, char *name) {
  for (int i = 0; i < device->data_count; i++) {
    if (strcmp(device->data_arr[i].name, name) == 0) {
      return &device->data_arr[i];
    }
  }
  return NULL;
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
