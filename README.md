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

## Testing

1. Clone the repository

```
git clone https://git.fit.vutbr.cz/xkocma09/IPK_projekt2.git
```

2. Make the project using `make`
3. Run ipkcpd on background

```
./ipkcpd -h localhost -p 2025 -m udp & ./ipkcpd -h localhost -p 2026 -m tcp &
```

4. Change mode of the skript and run it

```
chmod +x test.sh
./test.sh
```

5. See the result
6. Get pid of the server processes and send them SIGINT

```
ps 
...
123 ipkcpd pts/0 
124 ipkcpd pts/0 
...
kill -2 123
kill -2 124
```

## References
Ryšavý, Ondřej, Ph. D. “IPK-DemoUdp.” *FIT - VUT Brno - Git*, git.fit.vutbr.cz/NESFIT/IPK-Projekty/src/branch/master/Stubs/cpp/DemoUdp/server.c.<br>
Ryšavý, Ondřej, Ph. D. “IPK-DemoTcp.” *FIT - VUT Brno - Git*, git.fit.vutbr.cz/NESFIT/IPK-Projekty/src/branch/master/Stubs/cpp/DemoTcp/server.c.<br>
Kadam Patel. “Signals in C Language.” *GeeksforGeeks*, 8 Feb. 2018, www.geeksforgeeks.org/signals-c-language<br>
www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html