#ifndef MYSHELL_H
#define MYSHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_LINE_LENGTH 1024
#define MAX_ARGS 64
#define MAX_PATH_LENGTH 256
#define DELIMITERS " \t\r\n"


typedef struct {
    char *args[MAX_ARGS];
    int arg_count;
    char *input_file;
    char *output_file;
    int append_mode;
    int background;
} Command;


void shell_loop(FILE *input);
char *read_line(FILE *input);
Command *parse_line(char *line);
int execute_command(Command *cmd);
int execute_builtin(Command *cmd);
void free_command(Command *cmd);


int cmd_cd(Command *cmd);
int cmd_clr(Command *cmd);
int cmd_dir(Command *cmd);
int cmd_environ(Command *cmd);
int cmd_echo(Command *cmd);
int cmd_help(Command *cmd);
int cmd_pause(Command *cmd);
int cmd_quit(Command *cmd);

void show_cwd(void);
void handle_redirection(Command *cmd);
void restore_redirection(int saved_stdin, int saved_stdout);

extern char **environ;
extern char shell_path[MAX_PATH_LENGTH];

#endif
