
/* CSCI 347 micro-make
 *
 * 9 MAR 2018, Natasha Ng
 */

#ifndef _ARG_PARSE_H_
#define _ARG_PARSE_H_

/*PROTOTYPES*/

/* Arguments Parser
 * line    The input line to execute
 * argcp   The number of arguments
 * This function returns a new array of pointers
 */
char** arg_parse(char* line, int *argcp);

#endif