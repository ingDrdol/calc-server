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
#define BUF_LEN 255

#define ERR_MSG(x) loc_error_msg[x*(-1) - 1]

typedef double (*funcvar)(double, double);
extern int errno; 

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
        return 0;
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
double get_result(const char *prefix_eq){
    double (*operation)(double, double);
    double first_exp = prefix_eq[3] - 48, second_exp = prefix_eq[5] - 48; 
    operation = select_op(prefix_eq[0]);
    fprintf(stderr, "%c, %lf, %lf\n", prefix_eq[0], first_exp, second_exp);
    return operation(first_exp, second_exp);

}

int main(void){
    errno = 0;
    double result;

    result = get_result("(* 2 5)");
    printf("%lf\n", result);

    return 0;
}