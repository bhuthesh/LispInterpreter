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

typedef struct lval{
    int type;
    long num;
    char* err;
    char* sym;
    int count;
    struct lval** cell;
}lval;

enum {LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR};

enum {LERR_DIV_BY_ZERO, LERR_BAD_OP, LERR_BAD_NUM};

lval* lval_num(long x) {
    lval *v = (lval*) malloc(sizeof(lval));
    v->type = LVAL_NUM;
    v->num = x;
    return v;
}

lval* lval_err(char *m) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_ERR;
    v->err= malloc(strlen(m)+1);
    strcpy(v->err,m);
    return v;
    
}

lval* lval_sym(char *s)
{
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SYM;
    v->sym = malloc(strlen(s)+1);
    strcpy(v->sym, s);
    return v;
}

lval* lval_sexpr(void) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

void lval_del(lval* v) {
    if (v == NULL)
        return;
    switch (v->type) {
        case LVAL_NUM:
            break;
        case LVAL_ERR:
            free(v->err);
            break;
        case LVAL_SYM:
            free(v->sym);
            break;
        case LVAL_SEXPR:
            for (int i = 0; i < v->count; i++) {
                lval_del(v->cell[i]);
            }
            free(v->cell);
            break;
            
        default:
            break;
    }
    free(v);
}

lval* lval_add(lval* v, lval* x) {
    if (v != NULL) {
    v->count++;
    v->cell = realloc(v->cell, sizeof(lval *) * v->count);
    v->cell[v->count - 1] = x;
    }
    return v;
}


lval* lval_read_num(mpc_ast_t* t) {
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return (errno != ERANGE) ? lval_num(x) : lval_err("invalid number");
}

lval* lval_read(mpc_ast_t* t) {
    if (strstr(t->tag, "number")) {
        return lval_read_num(t);
    }
    
    if (strstr(t->tag, "symbol")) {
        return lval_sym(t->contents);
    }
    // if we are starting from root('>') or new sexpression create new lval
    lval *x = NULL;
    if (strcmp(t->tag, ">") == 0) {
        x = lval_sexpr();
    }
    
    if (strstr(t->tag, "sexpr")) {
        x = lval_sexpr();
    }
    
    // read and fill with all the following expressions
    
    for (int i = 0; i < t->children_num; i++) {
        if (strcmp(t->children[i]->contents, "(") == 0) { continue;}
        if (strcmp(t->children[i]->contents, ")") == 0) { continue;}
        if (strcmp(t->children[i]->contents, "{") == 0) { continue;}
        if (strcmp(t->children[i]->contents, "}") == 0) { continue;}
        if (strcmp(t->children[i]->tag, "regex") == 0) { continue;}
        x = lval_add(x, lval_read(t->children[i]));
    }
    
    return x;
}

void lval_print(lval* v);

void lval_expr_print(lval *v, char open, char close) {
    putchar(open);
    
    for (int i = 0; i < v->count; i++) {
        lval_print(v->cell[i]);
        putchar(' ');
    }
    putchar(close);
}

void lval_print(lval* v) {
    switch (v->type) {
        case LVAL_NUM:
            printf("%ld", v->num);
            break;
        case LVAL_ERR:
            printf("Error : %s\n", v->err);
            break;
        case LVAL_SYM:
            printf("%s", v->sym);
            break;
        case LVAL_SEXPR:
            lval_expr_print(v, '(', ')');
            break;
        default:
            break;
    }
}

void lval_println(lval* v) {
    lval_print(v);
    printf("\n");
}

#if 0
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
        return (errno != ERANGE) ? lval_num(x) : lval_err(LERR_BAD_NUM);
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
#endif

lval* lval_eval(lval* v);
lval* lval_take(lval* v, int i);
lval* lval_pop(lval* v, int i);
lval* builtin_op(lval* v, char *s);


lval* lval_eval_sexpr(lval *v) {
    
    for (int i = 0; i < v->count; i++) {
        v->cell[i] = lval_eval(v->cell[i]);
    }
    
    for (int i = 0; i < v->count; i++) {
        if (v->cell[i]->type == LVAL_ERR) { return lval_take(v, i);}
    }
    
    if (v->count == 0) return v;
    
    if (v->count == 1) { return lval_take(v, 0);}
    
    lval* f = lval_pop(v, 0);
    
    if (f->type != LVAL_SYM) {
        lval_del(f); lval_del(v);
        return lval_err("S-expression doesn't start with an operator");
    }
    
    lval* result = builtin_op(v, f->sym);
    lval_del(f);
    return result;
}

lval* lval_eval(lval* v) {
    // evaluate if its a s-expression
    if (v->type == LVAL_SEXPR)
        return lval_eval_sexpr(v);
    // others remain same
    return v;
}

lval* lval_take(lval* v, int i) {
    lval* x = lval_pop(v, i);
    lval_del(v);
    return x;
}

lval* lval_pop(lval* v, int i) {
    // find item at index i
    lval* x = v->cell[i];
    memmove(&v->cell[i], &v->cell[i+1], sizeof(lval*) * (v->count - i - 1));
    v->count--;
    v->cell = realloc(v->cell, sizeof(lval*) * v->count);
    return x;
}
lval* builtin_op(lval* v, char* op) {
    for (int i = 0; i < v->count; i++)
        if (v->cell[i]->type != LVAL_NUM) {
            lval_del(v);
            return lval_err("non number lval");
        }
    lval* x = lval_pop(v, 0);
    if (strcmp(x->sym, "-") == 0 && v->count == 0) { x->num = -x->num;}
    while (v->count > 0){
        lval* y = lval_pop(v, 0);
        if (strcmp(op, "+") == 0) { x->num += y->num; }
        if (strcmp(op, "-") == 0) { x->num -= y->num; }
        if (strcmp(op, "*") == 0) { x->num *= y->num; }
        if (strcmp(op, "/") == 0) {
            if (y->num == 0) {
                lval_del(x); lval_del(y);
                x = lval_err("Division by zero!"); break;
            }
            else {
                x->num /= y->num;
            }
        }
        /* Delete element now finished with */
        lval_del(y);
    }
    lval_del(v);
    
    return x;
}



int main(int argc, const char * argv[])
{
    
    // create some parsers
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Symbol = mpc_new("symbol");
    mpc_parser_t* Sexpr = mpc_new("sexpr");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Lispy = mpc_new("lispy");
    
    mpca_lang(MPC_LANG_DEFAULT,
              "                                             \
              number : /-?[0-9]+/ ;                         \
              symbol : '+' | '-' | '*' | '/' ;              \
              sexpr : '(' <expr>* ')'     ;                  \
              expr : <number> | <symbol> | <sexpr>;         \
              lispy : /^/ <expr>* /$/ ;          \
              ",
              Number, Symbol, Sexpr, Expr, Lispy);
    
    //
    
    //    mpc_parser_t* Number = mpc_new("number");
    //    mpc_parser_t* Symbol = mpc_new("symbol");
    //    mpc_parser_t* Sexpr  = mpc_new("sexpr");
    //    mpc_parser_t* Expr   = mpc_new("expr");
    //    mpc_parser_t* Lispy  = mpc_new("lispy");
    //
    //    mpca_lang(MPC_LANG_DEFAULT,
    //              "                                          \
    //              number : /-?[0-9]+/ ;                    \
    //              symbol : '+' | '-' | '*' | '/' ;         \
    //              sexpr  : '(' <expr>* ')' ;               \
    //              expr   : <number> | <symbol> | <sexpr> ; \
    //              lispy  : /^/ <expr>* /$/ ;               \
    //              ",
    //              Number, Symbol, Sexpr, Expr, Lispy);
    puts("Lispy Interprter v 0.01. ");
    puts("Press C-c to exit\n");
    
    while (1) {
        // parse the input
        char* input = readline("lispy>");
        add_history(input);
        //printf("you entered %s\n", input);
        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lispy, &r)) {
            
            //lval result = eval(r.output);
            lval* result = lval_eval(lval_read(r.output));
            
            lval_println(result);
            lval_del(result);
            mpc_ast_delete(r.output);
        }
        else {
            
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }
        free(input);
    }
    
    mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lispy);
    
    return 0;
}

