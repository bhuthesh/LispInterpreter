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
char* readline(char* prompt) {
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

typedef struct {
    int type;
    long num;
    int err;
}lval;

enum {LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR};

enum {LERR_DIV_BY_ZERO, LERR_BAD_OP, LERR_BAD_NUM};

lval lval_num(long x) {
    lval v;
    v.type = LVAL_NUM;
    v.num = x;
    
    return v;
}

lval lval_err(int x) {
    lval v;
    v.type = LVAL_ERR;
    v.err = x;
    return v;
}

void lval_print(lval v) {
    switch (v.type) {
        case LVAL_NUM:
            printf("%ld", v.num);
            break;
        case LVAL_ERR:
            if (v.err == LERR_DIV_BY_ZERO) {
                printf("Error: division by zero!");
            }
            if (v.err == LERR_BAD_OP) {
                printf("Error: bad operator");
            }
            if (v.err == LERR_BAD_NUM) {
                printf("Error:bad number");
            }
        default:
            break;
    }
}

void lval_println(lval v) {
    lval_print(v);
    printf("\n");
}


lval eval_op(lval x, char* op, lval y) {
    if (x.type == LVAL_ERR) {
        return x;
    }
    
    if (y.type == LVAL_ERR) {
        return y;
    }
    
    
    if (strstr(op, "+")) {
        return lval_num(x.num + y.num);
    }
    if (strstr(op, "-")) {
        return lval_num(x.num - y.num);
    }
    if (strstr(op, "*")) {
        return lval_num(x.num * y.num);
    }
    if (strstr(op, "/")) {
        if (y.num != 0)
            return lval_num(x.num / y.num);
        else {
            return lval_err(LERR_DIV_BY_ZERO);
        }
    }
    return lval_err(LERR_DIV_BY_ZERO);
}

lval eval(mpc_ast_t* t) {
    if (strstr(t->tag, "number")) {
        errno = 0;
        long x = strtol(t->contents, NULL, 10);
        return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
    }
    char* op = t->children[1]->contents;
    lval x = eval(t->children[2]);
    
    int i = 3;
    while (strstr(t->children[i]->tag,"expr")) {
        x = eval_op(x, op, eval(t->children[i]));
        if (i < t->children_num) {
            i++;
        }
        else break;
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
              number : /-?[0-9]+/ ;                         \
              operator : '+' | '-' | '*' | '/' ;            \
              expr : <number> | '('<operator> <expr>+')';   \
              lispy : /^/ <operator> <expr>+ /$/ ;  \
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
            
            lval result = eval(r.output);
            lval_println(result);
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

