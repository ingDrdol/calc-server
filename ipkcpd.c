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
#define OP_POZ 1
#define EXPR_OFFSET 3*sizeof(char)
#define SP_OFFSET(x) x*sizeof(char)

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
    printf("%s\n", rest);
    read_check = sscanf(rest, "%c%[^\n]", &close, rest);
    if(close != ')'){
        errno = ERR_CLOSB;
        return -1;
    }
    printf("%s, %c\n", rest, close);
    return operation(first_exp, second_exp);

}

int main(void){
    errno = 0;
    char rest[BUF_LEN];
    const char *test_string = {"(* (+ 1 (+ 1 1)) (- 10 (/ 10 2)"};
    double result;
    result = get_result(test_string, &rest[0]);
    if(errno != 0)
        fprintf(stderr, "%s\n", ERR_MSG(errno));
    else
        printf("%lf\n", result);
    return 0;
}