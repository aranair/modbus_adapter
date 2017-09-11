#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus.h>

#ifdef _WIN32
# include <winsock2.h>
#else
# include <sys/socket.h>
#endif

#define MODBUS_TCP_MAX_ADU_LENGTH  260
#define MODBUS_RTU_MAX_ADU_LENGTH  256
#define SERVER_ID 1

enum {
  TCP,
  RTU
};

int main(int argc, char*argv[])
{
  modbus_t *ctx;
  uint8_t *query;
  int rc;
  int use_backend;
  int s = -1;
  int header_length;

  if (argc > 1) {
    if (strcmp(argv[1], "tcp") == 0) {
      use_backend = TCP;
    } else if (strcmp(argv[1], "rtu") == 0) {
      use_backend = RTU;
    } else {
      return -1;
    }
  } else {
    /* Use TCP by default for testing */
    use_backend = TCP;
  }

  /* Prepare a Modbus mapping with 2 holding registers
     (plus no output coil, one input coil and two input registers)
     This will also automatically set the value of each register to 0 */
  modbus_mapping_t *mapping = modbus_mapping_new(1, 1, 1, 1);
  if (!mapping) {
    fprintf(stderr, "Failed to allocate the mapping: %s\n", modbus_strerror(errno));
    exit(1);
  }

  /* Set some random values to registers */
  mapping->tab_registers[0] = 20;
  mapping->tab_bits[0] = 1;

  if (use_backend == TCP) {
    ctx = modbus_new_tcp("127.0.0.1", 1502);
    query = malloc(MODBUS_TCP_MAX_ADU_LENGTH);
  } else {
    ctx = modbus_new_rtu("/dev/ttys028", 115200, 'N', 8, 1);
    modbus_set_slave(ctx, SERVER_ID);
    query = malloc(MODBUS_RTU_MAX_ADU_LENGTH);
  }

  header_length = modbus_get_header_length(ctx);
  /* modbus_set_debug(ctx, TRUE); */

  if (!ctx) {
    fprintf(stderr, "Failed to create the context: %s\n", modbus_strerror(errno));
    exit(1);
  }

  header_length = modbus_get_header_length(ctx);
  modbus_set_debug(ctx, TRUE);

  if (use_backend == TCP) {
    s = modbus_tcp_listen(ctx, 1);
    modbus_tcp_accept(ctx, &s);
  } else {
    rc = modbus_connect(ctx);
    if (rc == -1) {
      fprintf(stderr, "Unable to connect %s\n", modbus_strerror(errno));
      modbus_free(ctx);
      return -1;
    }
  }

  /* length of the request/response */
  int len;
  for (;;) {
    int rc;
    rc = modbus_receive(ctx, query);

    /* query size */
    if (rc > 0) {
      modbus_reply(ctx, query, rc, mapping);
    } else if (rc == -1) {
      /* Connection closed by the client or error */
      break;
    }
  }
  printf("Exit the loop: %s\n", modbus_strerror(errno));

  modbus_mapping_free(mapping);
  modbus_close(ctx);
  modbus_free(ctx);
}
