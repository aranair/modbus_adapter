What is this?
------------------

- Demo program written in plain C that:
  - Speaks the modbus protocol using [libmodbus][2]
  - Reads information from a PLC via an inverter over modbus RTU
  - Relays information to a Kepware Server via Modbus TCP/IP
- Tested on OSX Sierra 10.12.6 and Windows 10

## Re-defining Connections

- libconfig[1] is used to define the modbus connections
- Can be easily changed to work with whatever device you need to connect to either via Modbus RTU or Modbus TCP/IP.

## Sample config.cfg

You may find a sample of a config file here: [https://github.com/aranair/modbus_adapter/blob/master/config.cfg][cfg]

## Virtual Serial Ports via Pseudo Terminal

If you want to test RTU mode using pseudo terminal on localhost:

- `brew install socat`
- `socat -d -d pty,raw,echo=0 pty,raw,echo=0` to get two pseudo terminals assigned.
- `cat < /dev/ttys035>` on a new terminal 2
- `echo "Test" > /dev/ttys037` on another new terminal 3 to see the results on terminal 2

## Building it from source

*OSX*

```
gcc slave_server.c -o slave_server `pkg-config --libs --cflags libmodbus`
gcc master_client.c -o master_client `pkg-config --libs --cflags libmodbus`
```

*Windows*

- Visual Studio 2017
- Open the solution file in the win32 folder


## Dependencies

- [libconfig][1]
- [libmodbus][2]

## License

MIT


[1]: https://github.com/hyperrealm/libconfig
[2]: https://github.com/stephane/libmodbus
[cfg]: https://github.com/aranair/modbus_adapter/blob/master/config.cfg
