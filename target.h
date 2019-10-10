
/* CSCI 347 micro-make
 *
 * 9 MAR 2018, Natasha Ng
 */

#ifndef _TARGET_H_
#define _TARGET_H_

#define EMPTY NULL

#include <stdlib.h>

struct target_st;
typedef struct target_st target;
//typedef struct target* target_list;

struct list_node;
typedef struct list_node node;
typedef struct list_node* List;

target* new_target (char* name);
target* find_target(char* name);

void add_dependency_target(target* tgt, char* dep);
void add_rule_target(target* tgt, char* rule);

typedef void(*list_action)(char*);

void for_each_rule(target *tgt, list_action action);
void for_each_dependency(target *tgt, list_action action);

void free_targets();

#endif