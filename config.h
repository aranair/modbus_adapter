#include <modbus.h>

enum ModbusType { TCP, RTU };
enum ModbusDataType { REG, COIL };
enum ModbusReadWrite { READ, WRITE };

struct ModbusConn
{
  modbus_t *ctx;
  enum ModbusType type;

  union {
    const char* rtu_port;
    const char* ip;
  };

  union {
    int baud;
    int port;
  };
};

struct ModbusData
{
  enum ModbusDataType type;
  int size;
  int address;
  const char* name;
};

struct ModbusDevice
{
  int address;
  int data_count;
  const char* name;
  struct ModbusData *data_arr;
  struct ModbusConn *conn;
};

struct ModbusConfig
{
  struct ModbusDevice *devices;
  int device_count;
};

void print_configs(struct ModbusDevice *devices)
{
  for(int i = 0; i < 2; i++) {
    printf("Device name: %s\n", devices[i].name);
    printf("Device address: %d\n", devices[i].address);
    printf("\tConn ip: %s\n", devices[i].conn->ip);
    printf("\tConn port: %d\n", devices[i].conn->port);

    for (int d = 0; d < devices[i].data_count; d++) {
      printf("\tData name: %s\n", devices[i].data_arr[d].name);
    }
    printf("\n");
  }
}

struct ModbusConfig * parse_config_devices()
{
  config_t cfg;
  config_setting_t *setting;

  config_init(&cfg);

  /* Read the file. If there is an error, report it and exit. */
  if(! config_read_file(&cfg, "config.cfg"))
  {
    fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
    config_destroy(&cfg);
    return NULL;
  }

  struct ModbusConfig *config = malloc(sizeof(struct ModbusConfig));
  struct ModbusConn *conn_arr;
  conn_arr = malloc(sizeof(struct ModbusConn) * 2);

  setting = config_lookup(&cfg, "connections.rtu");
  if(setting != NULL)
  {
    const char *rtu_port;
    int baud;
    config_setting_lookup_string(setting, "rtu_port", &rtu_port);
    config_setting_lookup_int(setting, "baud", &baud);

    conn_arr[0].rtu_port = strdup(rtu_port);
    conn_arr[0].baud = baud;
    conn_arr[0].ctx = modbus_new_rtu(rtu_port, baud, 'N', 8, 1);
  }

  setting = config_lookup(&cfg, "connections.tcp");
  if(setting != NULL)
  {
    const char *ip;
    int port;
    config_setting_lookup_string(setting, "ip", &ip);
    config_setting_lookup_int(setting, "port", &port);

    conn_arr[1].ip = strdup(ip);
    conn_arr[1].port = port;
    conn_arr[1].ctx = modbus_new_tcp(ip, port);
  }

  setting = config_lookup(&cfg, "devices");

  if(setting == NULL) {
    return NULL;
  }

  struct ModbusDevice *device_arr;

  int count = config_setting_length(setting);
  device_arr = malloc(sizeof(struct ModbusDevice) * count);
  config->device_count = count;

  /* Loop through devices */
  const char *name;
  int connection;

  for(int i = 0; i < count; i++)
  {
    config_setting_t *device = config_setting_get_elem(setting, i);

    /* Parse attributes of a device */
    config_setting_lookup_int(device, "address", &device_arr[i].address);
    config_setting_lookup_string(device, "name", &name);
    config_setting_lookup_int(device, "connection", &connection);

    device_arr[i].name = strdup(name);
    device_arr[i].conn = &conn_arr[connection];

    /* Parse data */
    config_setting_t *data = config_setting_get_member(device, "data");
    int data_count = config_setting_length(data);
    device_arr[i].data_count = data_count;

    /* Assign memory for data_arr struct */
    device_arr[i].data_arr = (struct ModbusData *) malloc(sizeof(struct ModbusData) * data_count);

    /* Loop through data */
    for(int d = 0; d < data_count; d++) {
      config_setting_t *wd = config_setting_get_elem(data, d);

      const char *type;
      config_setting_lookup_string(wd, "type", &type);
      config_setting_lookup_string(wd, "name", &device_arr[i].data_arr[d].name);
      config_setting_lookup_int(wd, "size", &device_arr[i].data_arr[d].size);
      config_setting_lookup_int(wd, "address", &device_arr[i].data_arr[d].address);

      if (strcmp(type, "coil") == 0) {
        device_arr[i].data_arr[d].type = COIL;
      } else if (strcmp(type, "register") == 0) {
        device_arr[i].data_arr[d].type = REG;
      }
    }
  }
  config_destroy(&cfg);
  config->devices = device_arr;

  return config;
}
