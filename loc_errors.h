#ifndef LOC_ERRORS
    #define LOC_ERORS
    #include <stdio.h>

    #define ERR_DIV -1
    #define ERR_OPENB -2
    #define ERR_CLOSB -3
    #define ERR_OP -4

    const char *loc_error_msg[] = {"Division by zero", "Missing open bracelet", "Missing close bracelet", "Unknown operator"};

#endif