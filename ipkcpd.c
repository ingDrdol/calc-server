/*
*
* IPK projekt 1
*
* prijmeni, jmeno: Kocman, Vojta
* login: xkocma09
* soubor: ipkcpd.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <netdb.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "loc_errors.h"

#define DEBUG
#define UDP_MODE 0
#define TCP_MODE 1
#define BUF_LEN 255
#define COMM_LEN 10
#define OP_POZ 1
#define EXPR_OFFSET 3*sizeof(char)
#define SP_OFFSET(x) x*sizeof(char)
#define MAX_HOST_NAME_LEN 255
#define ARG_NUM 6
#define WELL_KNOWN_PORTS 1023 //number of well known ports
#define MAX_PORT_NUM 65353 //max port number for tcp/udp protocols
#define REQ_OFFSET 2
#define RES_OFFSET 3
#define MAX_TCP_CLIENTS 5

#define OPCODE(msg) msg[0]
#define REQ_LEN(msg) msg[1]
#define RES_LEN(msg) msg[2]
#define RES_STATUS(msg) msg[1]
#define ERR_MSG(x) loc_error_msg[x*(-1) - 1]

typedef double (*funcvar)(double, double);
extern int errno; 
int port_num, mode, sock_type, ssocket, wsocket, csocket;
char host_name[MAX_HOST_NAME_LEN];
pid_t is_child = 1;

double add(double x, double y){
    return x + y;
} 

double sub(double x, double y){
    return x - y;
}

double mul(double x, double y){
    return x * y;
}

double my_div(double x, double y){
    if(y == 0){
        errno = ERR_DIV;
        return -1;
    }
    return x / y;
}

/**
 * @brief selects function to be used based od argument operator
 * @param operator operator identifying function
 * @returns pointe to function on succes, if operator is not recognized errno is set to ERR_OP and NULL pointer is returned 
*/
funcvar select_op(char operator){
    switch(operator){
        case '+':
            return add;
        case '-':
            return sub;
        case '*':
            return mul;
        case '/':
            return my_div;
    }
    errno = ERR_OP;
    return NULL;
}

/**
 * @brief computes the result of equation given by arqument prefix_eq
 * @param prefix_eq pointer to a string containig the equation in prefix form
 *
 * @returns computed value
*/
double get_result(const char *prefix_eq, char *rest){
    int read_check, i;
    char close;
    double first_exp, second_exp = 5;
    double (*operation)(double, double) = select_op(prefix_eq[OP_POZ]);
    if(errno != 0)
        return -1;

    if(prefix_eq[0] != '('){
        errno = ERR_OPENB;
        return -1;
    }

 ///////////////////////////////////////////////////////////////////////////////////
    read_check = sscanf(prefix_eq + EXPR_OFFSET, "%lf %[^'\n']", &first_exp, rest);
    if(read_check != 2)
        first_exp = get_result(prefix_eq + EXPR_OFFSET, rest);
    if(errno != 0)
        return -1;
 /////////////////////////////////^^first value^^///////////////////////////////////
 /////////////////////////////////////////////////////////////////////////////////////
    read_check = sscanf(rest, " %lf%[^\n]", &second_exp, rest);
    if(read_check == 1){
        errno = ERR_CLOSB;
        return -1;
    }
    if(read_check != 2){
        i = 0;
        while(rest[i] != '(' && rest[i] != '\0')
            i++;
        if(rest[i] == '\0'){
            errno = ERR_OPENB;
            return -1;
        }
        second_exp = get_result(rest + SP_OFFSET(i), rest);
    }
    if(errno != 0)
        return -1;  
 ////////////////////////////////^^second value^^/////////////////////////////////////
    read_check = sscanf(rest, "%c%[^\n]", &close, rest);
    if(close != ')'){
        errno = ERR_CLOSB;
        return -1;
    }
    return operation(first_exp, second_exp);

}

/*********************************************************************
 * Title: Example of sigint handler
 * Author: Kadam Patel
 * Date: 08. 02. 2018
 * Availibility: https://www.geeksforgeeks.org/signals-c-language/
 ********************************************************************/ 
 ///////////////////////////////////////////////////////////////////////////
void handle_sigint(){  
  const char *buff = "BYE\n";                                                  
  if(is_child){
    close(wsocket);
  }                                       
  else{
    send(csocket, buff, strlen(buff), 0);
    if (errno != 0){
      fprintf(stderr, "%s\n", strerror(errno));
      exit(errno);
    }
    close(csocket);
  }
    
  exit(2);
}
 ////////////////////^^zpracovani signalu 2 'SIGINT'^^//////////////////////

/******************************************************************************************
 * Title: Example of getopt()
 * Author: unknown
 * Availability: https://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html
 *****************************************************************************************/
void load_arguments(int argc, char **argv){
  int c;
    while ((c = getopt(argc, argv, "h:p:m:")) != -1){
    switch (c)
      {
      case 'h':
        strcpy(host_name, optarg);
        break;
      case 'p':
        port_num = atoi(optarg);
        //////////////////////////////////////////////////////////////////////////////////////////////
        if(port_num <= WELL_KNOWN_PORTS || port_num >= MAX_PORT_NUM){
          fprintf(stderr, "ERROR: invalid port number '%d' must be within <1024, 65_353>\n", port_num);
          exit(1);
        }
        ////////////////////////////////^^kontrola cisla portu^^///////////////////////////////////////
        break;
      case 'm':
        if(!strcmp(optarg, "udp")){
          mode = UDP_MODE;
          sock_type = SOCK_DGRAM;
        }
        else if(!strcmp(optarg, "tcp")){
          signal(SIGINT, handle_sigint);
          mode = TCP_MODE;
          sock_type = SOCK_STREAM;
        }
        else{
        /////////////////////////////////////////////////////////////////////////////////
          fprintf(stderr, "ERROR: unknown mode expected [udp tcp] given %s\n", optarg);
          exit(1);
        ////////////////////////////^^neznamy protokol^^//////////////////////////////////
        }
        break;
      case '?':
	///////////////////////////////////////////////////////////////////////
        if (optopt == 'h')
          fprintf (stderr, "Option -%c requires a hostname.\n", optopt);
        else if (optopt == 'p')
          fprintf (stderr, "Option -%c requires a port number.\n", optopt);
        else if (optopt == 'm')
          fprintf (stderr, "Option -%c requires a mode.\n", optopt);
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);       
       // print_help();
        exit(1);
        break;
      default:
    //    print_help();
        exit(1);
        //////////////////^^neznamy, ci nevalidni prepinac^^///////////////////
      }
    } //end while
}

/*********************************************************************************************************
 * Title: Demonstration of trivial UDP communication
 * Author: Ondrej Rysavy (rysavy@fit.vutbr.cz)
 * Availability: https://git.fit.vutbr.cz/NESFIT/IPK-Projekty/src/branch/master/Stubs/cpp/DemoUdp/server.c
 ********************************************************************************************************/
void udp_server(){
  char rest[BUF_LEN];
  char buff[BUF_LEN];
  double result;
  socklen_t clientlen;
  struct sockaddr_in server_address, client_address;
  struct hostent *server;
  int optval = 1;

  ssocket = socket(AF_INET, SOCK_DGRAM, 0);
  if (errno != 0){
    fprintf(stderr, "%s\n", strerror(errno));
    exit(errno);
  }

    setsockopt(ssocket, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

  if ((server = gethostbyname(host_name)) == NULL) {
        fprintf(stderr,"ERROR: unknown hostname '%s'\n", host_name);
        exit(1);
  }

  bzero((char *) &server_address, sizeof(server_address));
  server_address.sin_family = AF_INET;
  bcopy((char *)server->h_addr_list[0], (char *)&server_address.sin_addr.s_addr, server->h_length);
  server_address.sin_port = htons((unsigned short)port_num);
	
  bind(ssocket, (struct sockaddr *) &server_address, sizeof(server_address)) ;
  
  if (errno != 0){
    fprintf(stderr, "%s\n", strerror(errno));
    exit(errno);
  }
    
    while(1) 
    {   
        clientlen = sizeof(client_address);
        recvfrom(ssocket, buff, BUF_LEN, 0, (struct sockaddr *) &client_address, &clientlen);
        if (errno != 0){
          fprintf(stderr, "%s\n", strerror(errno));
          exit(errno);
        }

        if(OPCODE(buff) == 0){
          result = get_result(buff + REQ_OFFSET, rest);
          if(errno == 0){
            OPCODE(buff) = 1;
            RES_STATUS(buff) = 0;
            sprintf(buff + RES_OFFSET, "%lf%c", result, '\0');
            RES_LEN(buff) = strlen(buff + RES_OFFSET);
          }
          else{
            OPCODE(buff) = 1;
            RES_STATUS(buff) = 1;
            sprintf(buff + RES_OFFSET, "%s%c", ERR_MSG(errno), '\0');
            RES_LEN(buff) = strlen(buff + RES_OFFSET);
            errno = 0;
          }
          
        }
        else{
          OPCODE(buff) = 1;
          RES_STATUS(buff) = 1;
          sprintf(buff + RES_OFFSET, "Wrong opcode%c", '\0');
        }

        sendto(ssocket, buff, strlen(buff + RES_OFFSET) + RES_OFFSET, 0, (struct sockaddr *) &client_address, clientlen);
        if (errno != 0){
          fprintf(stderr, "%s\n", strerror(errno));
          exit(errno);
        }
    }
}

void tcp_child(){
  char buff[BUF_LEN];
  char comm[COMM_LEN];
  char rest[BUF_LEN];
  double result;
  int load;

  recv(csocket, buff, BUF_LEN, 0);
  if (errno != 0){
    fprintf(stderr, "%s\n", strerror(errno));
    exit(errno);
  }
  
  if(strcmp(buff, "HELLO\n") != 0){
    strcpy(buff, "BYE\n");
    send(csocket, buff, strlen(buff), 0);
    if (errno != 0){
      fprintf(stderr, "%s\n", strerror(errno));
      exit(errno);
    }
  }

  strcpy(buff, "HELLO\n");

  send(csocket, buff, strlen(buff), 0);
  if (errno != 0){
    fprintf(stderr, "%s\n", strerror(errno));
    exit(errno);
  }
  
  while(strcmp(buff, "BYE\n") != 0){
    recv(csocket, buff, BUF_LEN, 0);
    if (errno != 0){
      fprintf(stderr, "%s\n", strerror(errno));
      exit(errno);
    }
    load = sscanf(buff, "%s %[^\n]", comm, buff);
    if(load == 2 && strcmp(comm, "SOLVE") == 0){  
      result = get_result(buff, rest);
      if(errno == 0)
        sprintf(buff, "RESULT %lf\n", result);
      else{
        strcpy(buff, "BYE\n");
        errno = 0;
      }
    }
    else{
      strcpy(buff, "BYE\n");
    }
    
    send(csocket, buff, strlen(buff), 0);
    if (errno != 0){
      fprintf(stderr, "%s\n", strerror(errno));
      exit(errno);
    }
  }
  close(csocket);
  exit(0);
}

/*********************************************************************************************************
 * Title: Demonstration of trivial UDP communication
 * Author: Ondrej Rysavy (rysavy@fit.vutbr.cz)
 * Availability: https://git.fit.vutbr.cz/NESFIT/IPK-Projekty/src/branch/master/Stubs/cpp/DemoTcp/server.c
 ********************************************************************************************************/
void tcp_welcome(){
	struct sockaddr_in sa;
	struct sockaddr_in sa_client;
  socklen_t sa_client_len=sizeof(sa_client);
  struct hostent *server;
	
  wsocket = socket(AF_INET, SOCK_STREAM, 0);
	if (errno != 0){
    fprintf(stderr, "%s\n", strerror(errno));
    exit(errno);
  }

	if ((server = gethostbyname(host_name)) == NULL) {
        fprintf(stderr,"ERROR: unknown hostname '%s'\n", host_name);
        exit(1);
  }

	bzero((char *) &sa, sizeof(sa));
  sa.sin_family = AF_INET;
  bcopy((char *)server->h_addr_list[0], (char *)&sa.sin_addr.s_addr, server->h_length);
  sa.sin_port = htons((unsigned short)port_num);
    
    
	bind(wsocket, (struct sockaddr*)&sa, sizeof(sa));
	if (errno != 0){
    fprintf(stderr, "%s\n", strerror(errno));
    exit(errno);
  }

	listen(wsocket, MAX_TCP_CLIENTS);
	if (errno != 0){
    fprintf(stderr, "%s\n", strerror(errno));
    exit(errno);
  }

	while(1)
	{
		csocket = accept(wsocket, (struct sockaddr*)&sa_client, &sa_client_len);		
		if (errno != 0){
      fprintf(stderr, "%s\n", strerror(errno));
      exit(errno);
    }
    is_child = fork();
    if(!is_child)
      tcp_child();
	}	
}

int main(int argc, char **argv){
  errno = 0;
  port_num = 0;
  mode = -1;
  sock_type = SOCK_DGRAM;

  //////////////////////////////////////
  if(argc != (ARG_NUM + 1)){
    //print_help();
    return 1;
  }
  ////^^kontrola poctu argumentu^^//////

  load_arguments(argc, argv);

  if(mode == UDP_MODE)
    udp_server();
  if(mode == TCP_MODE)
    tcp_welcome();  
  return 0;
}