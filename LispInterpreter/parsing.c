//
//  main.c
//  LispInterpreter
//
//  Created by bhuthesh r on 05/04/14.
//  Copyright (c) 2014 bhuthesh r. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

#ifdef __WIN32
#include <string.h>

static char buffer[2048];

// realine function for windows
char* reeadline(char* prompt) {
    fputs("lispy>", stdout);
    fgets(buffer, 2048, stdin);
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



int main(int argc, const char * argv[])
{

    puts("Lispy Interprter v 0.01. ");
    puts("Press C-c to exit\n");

    while (1) {
        char* input = readline("lispy>");
        add_history(input);
        printf("you entered %s\n", input);
        free(input);
    }
    
    return 0;
}

