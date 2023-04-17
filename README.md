# IPK-projekt 2

## usage

### console:

```
./ipkcpd -h [HOSTNAME] -p [PORTNUM] -m [MODE]
```

&emsp; HOSTNAME - domain name of hosting server<br>
&emsp; PORTNUM - valid non well-known port number <1024, 65 353><br>
&emsp; MODE - there are two supported modes "udp" and "tcp"<br>

## Modes

### UDP

Program accepts messages on given port. The message must have correct format described below. After reciving message opcode is validated if, for some reason, value in not 0 response status is set to 1 and `Wrong opcode` message is sent. When correct opcode is recived result is computed response status set to 0 and sent back to client end system.

#### message format
```
op = ['+','-','*','/']
expr = "(op expr expr)" or [[:digit:]]
%O = opcode - always 0
%l = length of message in bytes
expected format = "%O%l(op expr expr)"
```

### TCP

Program listens on givet port ready to recive connection requests. After request is accepted new socket is created and program creates child process using the `fork()` function. This child process is used to handle all client requests as long as client uses correct messages described below.
After BYE or unknown message is recieved child process sends BYE message as a response closes socket and exits(using the `exit()` function).<br>
When `SIGINT` signal is recieved program closes welcome socket and exits with value of SIGINT (2). Thanks to the shared signal mask all child processes created by `ipkpd` recive same signal close their own sockets send BYE message to client systems and exits with same value.

#### recognized messages
```
"HELLO" - opens communication
"SOLVE (op expr expr)" - request
"BYE" - ends communication

"RESULT [[:digit:]]" - is send as a response
```