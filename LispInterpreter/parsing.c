//
//  main.c
//  LispInterpreter
//
//  Created by bhuthesh r on 05/04/14.
//  Copyright (c) 2014 bhuthesh r. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

#include "mpc.h"

#define INPUT_BUFFER_SIZE 2048


#ifdef __WIN32
#include <string.h>

static char buffer[INPUT_BUFFER_SIZE];

// realine function for windows
char* reeadline(char* prompt) {
    fputs("lispy>", stdout);
    fgets(buffer, INPUT_BUFFER_SIZE, stdin);
    char* cpy = malloc(strlen(buffer) + 1);
    strcpy(cpy, buffer);
    cpy[strlen(cpy) - 1] = '\0';
    return cpy;
}

void add_history(char* line){
    
}

#else

#include <editline/readline.h>

#endif

long eval_op(long x, char* op, long y) {
    if (strstr(op, "+")) {
        return x + y;
    }
    if (strstr(op, "-")) {
        return x - y;
    }
    if (strstr(op, "*")) {
        return x * y;
    }
    if (strstr(op, "/")) {
        if (y != 0)
            return x / y;
        else {
            printf("attempt to divide by zero: exiting...\n");
            return 0;
        }
    }
    return 0;
}

long eval(mpc_ast_t* t) {
    if (strstr(t->tag, "number")) {
        return atoi(t->contents);
    }
    char* op = t->children[1]->contents;
    long x = eval(t->children[2]);
    
    int i = 3;
    while (strstr(t->children[i]->tag,"expr")) {
        x = eval_op(x, op, eval(t->children[i]));
        i++;
    }
    return x;
}


int main(int argc, const char * argv[])
{
    
    // create some parsers
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Expression = mpc_new("expr");
    mpc_parser_t* Lispy = mpc_new("lispy");
    
    mpca_lang(MPC_LANG_DEFAULT,
              "                                             \
              number : /-?[0-9]+/ ;                       \
              operator : '+' | '-' | '*' | '/' ;          \
              expr : <number> | '('<operator> <expr>+')'; \
              lispy : /^/ <operator> <expr>+ /$/ ;        \
              ",
              Number, Operator, Expression, Lispy);
    
    puts("Lispy Interprter v 0.01. ");
    puts("Press C-c to exit\n");
    
    while (1) {
        // parse the input
        char* input = readline("lispy>");
        add_history(input);
        //printf("you entered %s\n", input);
        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lispy, &r)) {
            
            long result = eval(r.output);
            printf("%li\n", result);
            mpc_ast_delete(r.output);
        } else {
            
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }
        free(input);
    }
    
    mpc_cleanup(4, Number, Operator, Expression, Lispy);
    
    return 0;
}

