//
//  main.c
//  LispInterpreter
//
//  Created by bhuthesh r on 05/04/14.
//  Copyright (c) 2014 bhuthesh r. All rights reserved.
//

#include <stdio.h>

static char input[1024];

int main(int argc, const char * argv[])
{

    puts("Lispy Interprter v 0.01. ");
    puts("Press C-c to exit\n");

    while (1) {
        fputs("lipsy>", stdout);
        fgets(input, 1024, stdin);
        printf("Okay. You entered %s", input);
        
    }
    
    return 0;
}

