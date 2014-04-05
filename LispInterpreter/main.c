//
//  main.c
//  LispInterpreter
//
//  Created by bhuthesh r on 05/04/14.
//  Copyright (c) 2014 bhuthesh r. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>



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

