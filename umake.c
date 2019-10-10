
/* CSCI 347 micro-make
*
* 5 MAR 2018, Natasha Ng
*/


#include "arg_parse.h"
#include "target.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>

/* PROTOTYPES */

/* Process Line
* line   The command line to execute.
* This function interprets line as a command line.  It creates a new child
* process to execute the line and waits for that process to complete.
*/
void processline(char* line);

/* Recurse Deps(Dependencies)
* target_args  The dependencies for a target file
* Performs the recursion through a target's dependencies
*/
void recurse_deps(char* target_args);

/* Expand
* orig   The input string that may contain variables to be expanded
* new   An output buffer that will contain a copy of orig with all variables expanded
* newsize The size of the buffer pointed to by new.
* returns 1 upon success or 0 upon failure
*
* Example: "Hello ${PLACE}" will expanded to "Hello, World" for
* environment variable PLACE="WORLD"
*/
int expand(char* orig, char* new, int newsize);

/* redirect
* line  command line to execute
* This function decides which function for append, truncate, and redirects input
*/
int redirect(char* line);

/* append
 * line   command line to execute
 * A helper function to redirect, Performs << commands
*/
void append(char* line);

/* truncate
 * line   command line to execute
 * A helper function to redirect, Performs < commands
*/
void truncat(char* line);

/* doredirect
 * line   command line to execute
 * A helper function to redirect, Performs < and <..> commands
*/
void doredirect(char* line);

/* Main entry point.
* argc    A count of command-line arguments
* argv    The command-line argument valus
*
* Micro-make (umake) reads from the uMakefile in the current working
* directory.  The file is read one line at a time.  Lines with a leading tab
* character ('\t') are interpreted as a command and passed to processline minus
* the leading tab.
*/
int main(int argc, const char* argv[]) {

  if (fopen("./uMakefile", "r") == NULL) {
    fprintf(stderr, "error: no uMakefile\n");
    errno = EPERM;
  }
  else {
    FILE* makefile = fopen("./uMakefile", "r");
    size_t  bufsize = 0;
    char*   line    = NULL;
    ssize_t linelen = getline(&line, &bufsize, makefile);
    int args        = 0;
    target* tgt     = NULL;

    while(-1 != linelen) {

      if(line[linelen-1]=='\n') {
        linelen -= 1;
        line[linelen] = '\0';
      }

      /*Adding Target and dependencies*/
      if ( (strchr(line, ':') != NULL)  || (strchr(line, '=') !=NULL) ){

        if (strchr(line, ':') != NULL){
          *strchr(line, ':') = ' ';
        }
        else if (strchr(line, '=') != NULL) {

          char** env_assign = arg_parse(line, &args);

          // case: CFLAGS=#-g
          if (env_assign[1] == NULL) {
            free(env_assign);
          }
          else {
            if (setenv(env_assign[0], env_assign[1], 1) != 0)
            return 0;
            else
            setenv(env_assign[0], env_assign[1], 1);
            free(env_assign);
          }
        }

        char** dep = arg_parse(line, &args);
        tgt = new_target(dep[0]);

        for (int i = 1; i < args; i++) {
          if (dep[i] != NULL)
          add_dependency_target(tgt, strdup(dep[i]));
        }

        free(dep);
        args = 0;
      }

      // Adding Rules
      if(line[0] == '\t'){
        add_rule_target(tgt, strdup(&line[1]));
      }

      linelen = getline(&line, &bufsize, makefile);

    }

    // argc: number of arguments passed into umake program
    // argv: the arguments passed in (ie. C B C '\0')
    for (int i = 1; i < argc; i++) {
      char* target_args = (char*) argv[i];
      recurse_deps(target_args);
    }

    free(line);
    return EXIT_SUCCESS;
  }
}

/* Recurse dependencies */
void recurse_deps(char* target_args) {

  target* tgt = find_target(target_args);

  // target info

  struct stat tgtfile;
  int tgtstatus = lstat(target_args, &tgtfile);

  if (tgt != EMPTY) {

    // dependency info

    struct stat depfile;
    int depstatus = lstat(target_args, &depfile);

    for_each_dependency(tgt, recurse_deps);

    // Condition: rules are only execute if target file does not exist
    if (tgtstatus != 0 && depstatus != 0)
      for_each_rule(tgt, processline);
  }
}


/* Process Line */

void processline (char* line) {

  int newsize = 1024;
  char newBuf[newsize];

  if (expand(line, newBuf, newsize)) {
    size_t newsize_t = (size_t) newsize;
    line = strndup(newBuf, newsize_t);
  }

  if( (strchr(line, '>') != NULL) || (strchr(line, '<') != NULL) ) {
    redirect(line);
  }

  int argc = 0;
  char** argv = arg_parse(strdup(line), &argc);

  if (argc > 0) {

    const pid_t cpid = fork();
    switch(cpid) {

      case -1: {
        perror("fork");
        break;
      }

      case 0: {
        execvp(argv[0], argv);
        perror("execvp");
        exit(EXIT_FAILURE);
        break;
      }

      default: {
        int   status;
        const pid_t pid = wait(&status);
        if(-1 == pid) {
          perror("wait");
        }
        else if (pid != cpid) {
          fprintf(stderr, "wait: expected process %d, but waited for process %d",
          cpid, pid);
        }
        break;
      }
    }
  }
  free(argv);
}

/* expand: returns 1 for success, 0 for failure */
int expand(char* orig, char* new, int newsize) {

  int varbeg= 0; // beginning of var assign/expand
  int varend= 0; // end of var assign/expand
  int index = 0;
  int state = 1;
  int status= 0;

  int len = strlen(orig);

  for (int i = 0; i < len; i++) {
    // printf("state: %d, new: %s\n", state, new);

    switch(state) {

      // no variables
      case 1: {
        if (orig[i] == '$')
        state = 2;
        else if (orig[i] == '#')
        state = 5;
        // put non variable letters into new
        else {
          new[index] = orig[i];
          index++;
          new[index] = '\0';
        }
        break;
      }

      // handles $
      case 2: {
        if (orig[i] == '{')
        state = 3;
        else if (orig[i] == '$')
        state = 2;
        else if (orig[i] == '#')
        state = 5;

        // put non variable letters into new
        else {
          state = 1;
          new[index] = orig[i];
          index++;
          new[index] = '\0';
        }
        break;
      }

      // handles ${
      case 3: {
        if (orig[i] == '}')
        state = 1;
        else if (orig[i] == '#')
        state = 5;
        else {
          varbeg = i; // get current pos
          state = 4;
        }
        break;
      }

      // handles ${<var>}
      case 4: {
        if (orig[i] == '}') {

          state = 1;
          varend = i - 1;

          char var[varend - varbeg + 1];
          int vindex = 0;

          // parse variable letters from orig
          for (int j = varbeg; j <= varend; j++) {
            var[vindex] = orig[j];
            vindex++;
            var[vindex] = '\0';
          }

          // handle environment var assign
          if (getenv(var)!= NULL) {
            char* expanded = getenv(var);
            //printf("expanded:%s\n", expanded);

            // put expanded into new
            for (int e = 0; e < strlen(expanded); e++) {
              new[index] = expanded[e];
              index++;
              new[index] = '\0';
            }
            status = 1;
          }
        }
        status = 1;
        break;
      }

      // handles #
      case 5: {
        state = 5;
        break;
      }
    }
  }

  // handles ${<var>
  if (state == 4) {
    varend = len - 1;
    for (int o = varbeg-1; o <= varend; o++) {
      new[index] = orig[o];
      index++;
      new[index] = '\0';
    }
  }

  newsize = index;
  return status;

}

int redirect(char* line) {

  int state = 1;

  for (int i = 0; i < strlen(line); i++) {

    switch(state) {

      // input
    case 1: {
      if (line[i] == '>')
        state = 2;
      else if (line[i] == '<')
        state = 5;
      else
        state = 1;
      break;
    }

    // append >> or  truncate >
  case 2: {
    if (line[i] == '>')
      state = 3;
    else
      state = 4;
    break;
  }

    // append >>
    case 3: {
      state = 3;
      break;
    }

      // truncate >
    case 4: {
      state = 4;
      break;
    }

      // io redir ..<..
    case 5: {
      if (line[i] == '>')
        state = 6;
      else
        state = 5;
      break;
    }

      // io redir ..<..>..
    case 6: {
      state = 6;
      break;
    }
    }
  }

  if (state == 3)
    append(line);

  else if (state == 4)
    truncat(line);

  else if (state == 5)
    doredirect(line);

  else if (state == 6)
    doredirect(line);

  return 0;

}

void append(char* line){
  
  int out;
  int argc = 0;
  char** args = arg_parse(strdup(line), &argc);
  int app_sym_idx = -1;

  if (args > 0) {

    // parses position of out file and removes '>>'
    for (int i = 0; i < argc; i++) {
      if (*args[i] == '>') {
        app_sym_idx = i;
        args[i] = args[i+1];
      }
    }

    if (app_sym_idx != -1) {
      out = open("args[app_sym_idx]", O_CREAT | O_APPEND | O_RDWR, 0666);

      if (out < 0) {
        perror("open");
        exit(1);
      }

      dup2(out, 1);
      close(out);

      execlp("cat", "date", NULL);
      perror("execlp");
      exit(1);

    }
  }
  free(args);
}

void truncat(char* line){

  int in;
  int argc = 0;
  char** args = arg_parse (strdup(line), &argc);
  int trunc_sym_idx = -1;

  if (args > 0) {

    // parses and removes '<'
    for (int i = 0; i < argc; i++) {
      if (*args[i] == '<') {
        trunc_sym_idx = i;
        args[i] = args[i+1];
       }
    }
  }

  if (trunc_sym_idx != -1) {
    in = open("args[trunc_sym_idx]", O_CREAT | O_TRUNC | O_RDONLY, 0666);

    if (in < 0) {
      perror("open");
      exit(1);
    }

    dup2(in, 0);
    close(in);

    execlp("wc", "-a", "-l", NULL);
    perror("execlp");
    exit(1);

  }
  free(args);
}

void doredirect(char* line) {

  int in, out;
  int argc = 0;
  char** args = arg_parse(strdup(line), &argc); // includes '<' or '>'
  int in_sym_idx = -1;
  int out_sym_idx = -1;

  if (args > 0) {

    // parses position of in/out files and removes '<' or '>'
    for (int i = 0; i < argc; i++){
      if (*args[i] == '<'){
        in_sym_idx = i;
        args[i] = args[i+1];
      }
      else if (*args[i] == '>') {
        out_sym_idx = i;
        args[i] = args[i+1];
      }
    }

    // <..>
    if (out_sym_idx != -1) {

      in = open(args[in_sym_idx+1], O_RDONLY);

      if (in < 0) {
        perror("open");
        exit(1);
      }

      out = open(args[out_sym_idx+1], O_WRONLY | O_TRUNC | O_CREAT, 0666);

      if (out < 0) {
        perror("open");
        exit(1);
      }

      dup2(in, 0);
      dup2(out, 1);

       close(in);
       close(out);

      execvp(args[0], args);
      perror("execvp");
    }

    //.. <..
    else {

      in = open(args[in_sym_idx+1], O_RDONLY);

      if (in < 0) {
        perror("open");
        exit(1);
      }

      dup2(in, 0);
      close(in);

      execvp(args[0], args);
      perror("execvp");

    }
  }
  free(args);
}