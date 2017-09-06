## Setup

### Virtual Serial Ports via Pseudo Terminal

- `brew install socat`
- `socat -d -d pty,raw,echo=0 pty,raw,echo=0` to get two pseudo terminals assigned.
- `cat < /dev/ttys035>` on a new terminal 2
- `echo "Test" > /dev/ttys037` on another new terminal 3 to see the results on terminal 2

### Build

```
gcc slave_server.c -o slave_server `pkg-config --libs --cflags libmodbus`
gcc master_client.c -o master_client `pkg-config --libs --cflags libmodbus`
```
