#include <stdio.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus.h>
#include <stdbool.h>
#include <libconfig.h>

#include "utility.h"
#include "config.h"

void initialize_connection(modbus_t *ctx) {
  if (modbus_connect(ctx) == -1) {
    fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
    modbus_free(ctx);
    exit(1);
  }
}

struct ModbusDevice * find_device(struct ModbusConfig *config, char *name) {
  for (int i = 0; i < config->device_count; i++) {
    if (strcmp(config->devices[i].name, name) == 0) {
      return &config->devices[i];
    }
  }
  return NULL;
}

struct ModbusData * find_data(struct ModbusDevice *device, char *name) {
  for (int i = 0; i < device->data_count; i++) {
    if (strcmp(device->data_arr[i].name, name) == 0) {
      return &device->data_arr[i];
    }
  }
  return NULL;
}

int main(int argc, char*argv[])
{
  struct ModbusConfig *config = parse_config_devices();
  struct ModbusDevice *devices = config->devices;
  print_configs(devices);

  int mIterations = 0;

  struct ModbusDevice *plc = find_device(config, "hitachiwj200");
  struct ModbusDevice *kep = find_device(config, "kepware");

  modbus_t *plc_ctx;
  modbus_t *kep_ctx;

  plc_ctx = plc->conn->ctx;
  kep_ctx = kep->conn->ctx;

  if (!plc_ctx || !kep_ctx) {
    fprintf(stderr, "Failed to connect context: %s\n", modbus_strerror(errno));
    exit(1);
  }

  modbus_set_slave(plc_ctx, plc->address);
  modbus_set_slave(kep_ctx, kep->address);
  initialize_connection(plc_ctx);
  initialize_connection(kep_ctx);

  int write_freq_addr = find_data(plc, "write_freq")->address;
  int write_coil_addr = find_data(plc, "write_coil")->address;
  int read_freq_addr  = find_data(plc, "read_freq")->address;
  int read_coil_addr  = find_data(plc, "read_estop")->address;

  /* modbus_set_debug(ctx, TRUE); */

  int16_t real_freq;
  int8_t estop_coil;

  for (;;) {
    switch (mIterations) {
      case 2:
        set_speed(plc_ctx, write_freq_addr, 15);
        set_coil(plc_ctx, write_coil_addr, true);
        break;

      case 4:
        set_speed(plc_ctx, write_freq_addr, 25);
        break;

      case 6:
        set_speed(plc_ctx, write_freq_addr, 10);
        break;

      case 8:
        set_coil(plc_ctx, write_coil_addr, false);
        set_speed(plc_ctx, write_freq_addr, 0);
        break;

      case 10:
        mIterations = 0;
        break;

      default:
        break;
    }

    real_freq = read_register(plc_ctx, 0);
    /* printf("\nreal_freq: %i\n", real_freq); */

    estop_coil = read_coil(plc_ctx, 0);
    /* printf("coil: %i\n\n", estop_coil); */

    /* Relay values to Kepware via TCP */
    set_coil(kep_ctx, find_data(kep, "write_coil")->address, estop_coil);
    set_speed(kep_ctx, find_data(kep, "write_freq")->address, real_freq);

    mIterations++;
    sleep_ms(500);
  }

  printf("Exit the loop.\n");
  modbus_close(plc_ctx);
  modbus_free(plc_ctx);

  modbus_close(kep_ctx);
  modbus_free(kep_ctx);
}
