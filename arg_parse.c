
/* CSCI 347 micro-make
*
* 9 MAR 2018, Natasha Ng
*/

#include "arg_parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <string.h>

/*
* Arg Count, helper function
*/
static int arg_count (char* line) {

  int count = 0;
  int state = 1;
  int i = 0;

  while (line[i] != '\0' && line[i] != '#') {
    switch (state) {

      case 1: {
        if (isspace(line[i] && line[i] != '='))
        state = 1;
        else {
          state = 2;
          ++count;
        }
      }

      case 2: {
        if (isspace(line[i]))
        state = 1;
        else
        state = 2;
      }
    }
    ++i;

  }
  return count;
}

/*
* Arg Parse
*/
char** arg_parse(char* line, int *argcp) {
  
  if (strchr(line, '='))
  *strchr(line, '=') = ' ';

  int argc= arg_count(line);
  *argcp = argc;
  char** args = malloc ( (1+argc) * sizeof(char*) );

  int state = 1;
  int i = 0;
  int k = 0;

  while (line[i] != '\0' && line[i] != '#') {
    
    switch (state) {

      case 1: {
        if (isspace(line[i]))
        state = 1;
        else {
          state = 2;
          args[k] = &line[i];
          k++;
        }
        break;
      }

      case 2: {
        if (isspace(line[i]) ) {
          state = 1;
          line[i] = '\0';
        }
        else
        state = 2;
        break;
      }
    }

    ++i;
  }

  args[k] = NULL;
  return args;
}