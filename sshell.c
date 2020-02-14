#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define CMDLINE_MAX 512
#define AGRUMENT_MAX 16
#define PIPE_MAX 4

/*Data structure to represent the stack*/
struct stack {
  int length;
  char **list;
};
/*Initialziation of stack*/
struct stack *makeStack() {
  struct stack *new = malloc(sizeof(struct stack));
  new->length = 0;
  new->list = NULL;

  return new;
}
/*Free stack after being used*/
void freestk(struct stack *stk) {
  for (int i = 0; i < stk->length; i++) {
    free(stk->list[i]);
  }
  free(stk);
}
/*Helper function to push the directory on the stack*/
void push(struct stack *stk, char *content) {
  if (stk->length == 0) {
    stk->list = malloc(sizeof(char *));
    stk->length++;
  } else {
    stk->length++;
    stk->list = realloc(stk->list, (sizeof(char *) * (stk->length)));
  }

  stk->list[stk->length - 1] = malloc(sizeof(char) * strlen(content));
  strcpy(stk->list[stk->length - 1], content);
}
/*Helper function to pop the latest directory on the stack*/
void pop(struct stack *stk) {
  free(stk->list[stk->length - 1]);
  stk->length--;
}
/*The function to get te Command list without any character*/
void getCommandList(char ***commandlist, int *totalword, char *stringbefore) {
  char *token = strtok(stringbefore, " "); //Split the cmd by whitespace
  *commandlist = malloc((*totalword + 1) * sizeof(**commandlist));

  while (token) {
    (*commandlist)[*totalword] = malloc(sizeof(char) * (1 + strlen(token)));
    strcpy((*commandlist)[*totalword], token);
    (*totalword) += 1;
    token = strtok(NULL, " ");
    if (token) {
      *commandlist = realloc(*commandlist, sizeof(char *) * (*totalword + 1));
    }
  }

  *commandlist = realloc(*commandlist, sizeof(char *) * (*totalword + 1));
  (*commandlist)[*totalword] = NULL;
}
/*The function to get the Pipe list*/
void getPipeList(char ***pipe_list, int *pipe_commonds_num, int *num_of_pipe_character, char *stringbefore) {
  *pipe_list = malloc((*pipe_commonds_num + 1) * sizeof(**pipe_list));
  for (int i = 0; stringbefore[i] != '\0'; ++i) {
    if (stringbefore[i] == '|')
      ++(*num_of_pipe_character);
  }

  char *token_line = strtok(stringbefore, "|");
  while (token_line != NULL && token_line[0] == ' ') {
    token_line++;
  }

  while (token_line != NULL && token_line[strlen(token_line) - 1] == ' ') {
    token_line[strlen(token_line) - 1] = '\0';
  }

  while (token_line) {
    (*pipe_list)[*pipe_commonds_num] = malloc(sizeof(char) * (1 + strlen(token_line)));
    strcpy((*pipe_list)[*pipe_commonds_num], token_line);
    (*pipe_commonds_num) += 1;
    token_line = strtok(NULL, "|");
    while (token_line != NULL && token_line[0] == ' ') {
      token_line++;
    }
    while (token_line != NULL && token_line[strlen(token_line) - 1] == ' ') {
      token_line[strlen(token_line) - 1] = '\0';
    }
    if (token_line) {
      *pipe_list = realloc(*pipe_list, sizeof(char *) * (*pipe_commonds_num + 1));
    }
  }
}
int builtin(char *cmd, char **commandlist, struct stack *stk) {
  int retval = 0;
  if (!strcmp(commandlist[0], "pwd")) {
    char current_directory[CMDLINE_MAX];
    getcwd(current_directory, sizeof(current_directory));
    fprintf(stdout, "%s\n", getcwd(current_directory, sizeof(current_directory)));

    /* To implement cd */
  } else if (!strcmp(commandlist[0], "cd")) {
    int change = chdir(commandlist[1]);
    if (change != 0) {
      fprintf(stderr, "Error: no such directory\n");
      retval = 1;
      fprintf(stderr, "+ completed '%s' [%d]\n", cmd, retval);
      return 1;
    }
    retval = 0;
    /* To implement pushd */
  } else if (!strcmp(commandlist[0], "pushd")) {
    int change = chdir(commandlist[1]);
    if (change != 0) {
      fprintf(stderr, "Error: no such directory\n");
      retval = 1;
      fprintf(stderr, "+ completed '%s' [%d]\n", cmd, retval);
      return 1;
    } else {
      push(stk, commandlist[1]);
    }
    /* To implement popd */
  } else if (!strcmp(commandlist[0], "popd")) {
    if (stk->length > 0) {
      char *last = "..";
      chdir(last);
      pop(stk);
    } else {
      fprintf(stderr, "Error: directory stack empty\n");
      retval = 1;
      fprintf(stderr, "+ completed '%s' [%d]\n", cmd, retval);
      return 1;
    }
    /* To implement dirs */
  } else if (!strcmp(commandlist[0], "dirs")) {
    char current_directory[CMDLINE_MAX];
    getcwd(current_directory, sizeof(current_directory));
    size_t curr_size = 0;
    int i = 0;
    while (current_directory[i]) {
      curr_size++;
      i++;
    }
    char *toprint = malloc(sizeof(char) * curr_size);
    strcpy(toprint, current_directory);
    printf("%s\n", toprint);

    if (stk->length > 0) {
      for (int j = stk->length - 1; j >= 0; j--) {
        size_t truncate = strlen(stk->list[j]) + 1; // 1 for slash
        toprint[curr_size - truncate] = 0;

        printf("%s\n", toprint);
        curr_size -= truncate;
      }
    }
    free(toprint);

  }
  return retval;
}
/*Function to implement the non-buildin function*/
int nonbuiltin(char *redirectfile, char **commandlist, const char *if_error_redirection) {
  pid_t pid;
  int status;
  pid = fork();
  /* Child */
  if (pid == 0) {
    int fd1;
    if (redirectfile) {
      fd1 = open(redirectfile, O_WRONLY);
      if (if_error_redirection) {
        dup2(fd1, STDERR_FILENO);
      } else {
        dup2(fd1, STDOUT_FILENO);
      }
      close(fd1);
    }
    execvp(commandlist[0], commandlist);
    fprintf(stderr, "Error: command not found\n");
    exit(1);
    /* Parent */
  } else if (pid > 0) {
    waitpid(-1, &status, 0);
    return WEXITSTATUS(status);
  } else {
    perror("fork");
    exit(1);
  }
}

int main(void) {

  char cmd[CMDLINE_MAX];
  struct stack *stk = makeStack();

  while (1) {
    char *nl;
    /* Print prompt */
    fflush(stdout);
    printf("sshell$ ");
    /* Get command line */
    fgets(cmd, CMDLINE_MAX, stdin);
    if (cmd[0] == '\n') {
      continue;
    }
    if (!isatty(STDIN_FILENO)) {
      printf("%s", cmd);
      fflush(stdout);
    }
    nl = strchr(cmd, '\n');
    if (nl)
      *nl = '\0';

    /*Get a temp cmd to avoid modifying the original command*/
    char *temp = malloc(sizeof(char) * (strlen(cmd) + 1));
    strcpy(temp, cmd);
    char *redirectfile = NULL;
    /*To check if ampersand exist*/
    char *if_error_rediction = strstr(temp, "&");
    /*To check if error redirection is valid*/
    if (if_error_rediction) {
      if (temp[strlen(temp) - strlen(if_error_rediction) - 1] == '|' ||
          temp[strlen(temp) - strlen(if_error_rediction) - 1] == '>') {
        temp[strlen(temp) - strlen(if_error_rediction)] = ' ';

      } else {
        fprintf(stderr, "Error: Wrong error redirection\n");
        continue;
      }
    }
    char *if_meta_exist = strstr(temp, ">");
    char *if_piping = strstr(temp, "|");
    char *stringbefore = NULL;
    char *metatoken = NULL;
    /*Dealing with the error of missing command*/
    if (if_meta_exist) {
      metatoken = strtok(temp, ">");
      stringbefore = malloc(sizeof(char) * (strlen(metatoken) + 1));
      strcpy(stringbefore, metatoken);

      while (if_meta_exist[0] == ' ') {
        if_meta_exist++;
      }

      if (if_meta_exist[0] == '>') {
        fprintf(stderr, "Error: missing command\n");
        continue;
      }
      /*Get the string after the metatoken*/
      metatoken = strtok(NULL, "> ");
      char *if_mislocated = strtok(NULL, " ");
      if (if_mislocated) {
        fprintf(stderr, "Error: mislocated output redirection\n");
        continue;
      }
      /*Get the redirect output file*/
      if (metatoken) {
        redirectfile = malloc(sizeof(char) * strlen(metatoken) + 1);
        strcpy(redirectfile, metatoken);
        /*Error management*/
      } else {
        fprintf(stderr, "Error: no output file\n");
        continue;
      }
      /*Dealing with error of ot able to open the output file*/
      FILE *fd1;
      fd1 = fopen(redirectfile, "a");
      if (fd1 == NULL) {
        fprintf(stderr, "Error: cannot open output file\n");
        continue;
      }
      fclose(fd1);
    } else {
      metatoken = strtok(temp, ">");
      stringbefore = malloc(sizeof(char) * (strlen(metatoken) + 1));
      strcpy(stringbefore, metatoken);
    }

    while (stringbefore[strlen(stringbefore) - 1] == ' ') {
      stringbefore[strlen(stringbefore) - 1] = '\0';
    }

    int pipe_commonds_num = 0;
    int num_of_pipe_character = 0;
    char **pipe_list = NULL;
    /*Error management regarding the piping*/
    if (if_piping) {
      while (stringbefore[0] == ' ') {
        stringbefore++;
      }
      if (stringbefore[0] == '|') {
        fprintf(stderr, "Error: missing command\n");
        continue;
      }
      getPipeList(&pipe_list, &pipe_commonds_num, &num_of_pipe_character, stringbefore);
      if (pipe_commonds_num > PIPE_MAX) {
        fprintf(stderr, "Error: too many piping arguments\n");
        continue;
      }
      if (num_of_pipe_character != pipe_commonds_num - 1) {
        fprintf(stderr, "Error: missing command\n");
        continue;
      }

    }
    int totalword = 0;
    char **commandlist = NULL;
    int returnval = 0;

    if (!if_piping) {
      /*To get the commandList that could be passed into execvp*/
      getCommandList(&commandlist, &totalword, stringbefore);
      if (totalword > AGRUMENT_MAX) {
        fprintf(stderr, "Error: too many process arguments\n");
        continue;
      }
    }

    if (if_piping) {
      int signal[pipe_commonds_num];
      pid_t pid;
      int fd[(pipe_commonds_num - 1) * 2];

      for (int i = 0; i < (pipe_commonds_num - 1); i++) {
        pipe(fd + (i * 2));
      }

      for (int index = 0; index < pipe_commonds_num; index++) {
        totalword = 0;
        char *tempval = malloc(sizeof(char) * strlen(pipe_list[index]));
        strcpy(tempval, pipe_list[index]);
        getCommandList(&commandlist, &totalword, tempval);

        int rt = 0;

        pid = fork();
        if (pid == 0) {
          if (index < (pipe_commonds_num - 1)) {
            if (if_error_rediction) {
              dup2(fd[(index * 2) + 1], 2);
            } else {
              dup2(fd[(index * 2) + 1], 1);
            }
          }

          if (index > 0) {
            dup2(fd[(index * 2) - 2], 0);
          }

          for (int i = 0; i < 2 * (pipe_commonds_num - 1); i++) {
            close(fd[i]);
          }

          rt = execvp(commandlist[0], commandlist);
        } else if (pid < 0) {
          perror("fork");
          exit(1);
        }
        signal[index] = rt;
      }

      for (int i = 0; i < (pipe_commonds_num - 1) * 2; i++) {
        close(fd[i]);
      }
      char *test = NULL;
      fgets(test, CMDLINE_MAX, stdout);
      fprintf(stdout, "%s", test);
      while (waitpid(0, 0, 0) < 0);
      fprintf(stderr, "+ completed '%s' ", cmd);
      for (int i = 0; i < pipe_commonds_num; i++) {
        fprintf(stderr, "[%d]", signal[i]);
      }
      fprintf(stderr, "\n");
      /*Run the builtin command*/
    } else if (!strcmp(commandlist[0], "exit") || !strcmp(commandlist[0], "pwd")
        || !strcmp(commandlist[0], "cd") || !strcmp(commandlist[0], "pushd")
        || !strcmp(commandlist[0], "popd") || !strcmp(commandlist[0], "dirs")) {

      if (!strcmp(commandlist[0], "exit")) {
        fprintf(stderr, "Bye...\n");
        fprintf(stderr, "+ completed '%s' [%d]\n", cmd, returnval);
        break;
      }
      returnval = builtin(cmd, commandlist, stk);
      if (returnval == 1) {
        continue;
      }
      /*Run the nonbuilt in command*/
    } else {
      returnval = nonbuiltin(redirectfile, commandlist, if_error_rediction);
    }
    fprintf(stderr, "+ completed '%s' [%d]\n", cmd, returnval);
    /*Free the all allocated memory if any*/
    if (redirectfile) {
      free(redirectfile);
    }
    for (int k = 0; k < totalword; k++) {
      free(commandlist[k]);
    }
    free(commandlist);
  }

  freestk(stk);
  return EXIT_SUCCESS;
}