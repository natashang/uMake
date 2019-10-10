
/* CSCI 347 mirco-make
*
* 9 MAR 2018, Natasha Ng
*/

#include "target.h"
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

struct target_st{
  char* target_name;
  target* rest;
  List dependencies;
  List rules;
};

//typedef target* target_list;
static target* theTargets;

struct list_node {
  List rest;
  char* current;
};

/*
* New target
*    Creates a target
*    Gives it the name inputted
*    Appends it to the front of theTargets list
*    Both dependencies and rules do not have anything defined yet
*/
target* new_target (char* name) {
  target* tgt = malloc(sizeof(target));

  if (tgt != EMPTY) {
    tgt->target_name = strdup(name);
    tgt->rest = theTargets;
    tgt->dependencies = EMPTY;
    tgt->rules = EMPTY;
    theTargets = tgt;
  }
  return tgt;
}

/*
* Find target
*    Goes through theTargets list and returns target with given name
*/
target* find_target (char* name) {

  assert(name != EMPTY);

  target* tgt = EMPTY;
  target* theTargetsCopy = theTargets;

  while (theTargetsCopy != EMPTY) {

    if (0 == strcmp(theTargetsCopy->target_name, name)) {
      tgt = theTargetsCopy;
      break;
    }
    else {
      theTargetsCopy = theTargetsCopy->rest;
    }
  }
  return tgt;
}

/*
* Add Dependency Target
*    Adds a dependency to a target
*/
void add_dependency_target(target* tgt, char* dep) {

  assert(tgt != EMPTY);

  if (tgt != EMPTY) {
    List* list = &tgt->dependencies;

    while (*list != EMPTY) {
      list = &((*list)->rest);
    }

    List newDep = malloc(sizeof(node));

    newDep->rest = EMPTY;
    newDep->current = dep;

    *list = newDep;
  }
  else {
    errno = ENOMEM;
  }
}

/*
* Add Rule Target
*    Adds a rule to a target
*/
void add_rule_target(target* tgt, char* rule) {

  assert(tgt != EMPTY);
  if (tgt != EMPTY) {
    List* list = &tgt->rules;

    while(*list != EMPTY) {
      list = &((*list)->rest);
    }

    List newRule = malloc(sizeof(node));

    newRule->rest = EMPTY;
    newRule->current = rule;

    *list = newRule;
  }
  else {
    errno = ENOMEM;
  }

}

/*
* For each rule
*      Does the action for each of the target's rules
*/
void for_each_rule(target* tgt, list_action action) {

  assert (tgt!= EMPTY);
  List arule = tgt->rules;

  while (arule != EMPTY) {
    action(arule->current);
    arule = arule->rest;
  }
}

/*
* For each dependency
*      Recursively does the action for each of the target's dependencies
*/
void for_each_dependency(target* tgt, list_action action) {

  assert (tgt!= EMPTY);
  List adep = tgt->dependencies;

  while (adep != EMPTY) {
    action(adep->current);
    adep = adep->rest;
  }
}

/*
* Free Targets
*    Frees the memory space allocated for theTargets
*/
void free_targets() {
  free(theTargets);
}