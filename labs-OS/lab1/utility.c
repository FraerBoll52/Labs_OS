#include "myshell.h"


void show_cwd(void) {
    char cwd[MAX_PATH_LENGTH];

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("myshell:%s> ", cwd);
    } else {
        printf("myshell> ");
    }
    fflush(stdout);
}


char *read_line(FILE *input) {
    char *line = malloc(MAX_LINE_LENGTH);

    if (line == NULL) {
        fprintf(stderr, "Ошибка выделения памяти\n");
        exit(EXIT_FAILURE);
    }

    if (fgets(line, MAX_LINE_LENGTH, input) == NULL) {
        free(line);
        return NULL;
    }

    line[strcspn(line, "\n")] = '\0';

    return line;
}


Command *parse_line(char *line) {
    Command *cmd = malloc(sizeof(Command));
    char *token;
    char *line_copy = strdup(line);
    char *saveptr;

    if (cmd == NULL || line_copy == NULL) {
        fprintf(stderr, "Ошибка выделения памяти\n");
        exit(EXIT_FAILURE);
    }

    cmd->arg_count = 0;
    cmd->input_file = NULL;
    cmd->output_file = NULL;
    cmd->append_mode = 0;
    cmd->background = 0;

    token = strtok_r(line_copy, DELIMITERS, &saveptr);

    while (token != NULL) {
        if (strcmp(token, "<") == 0) {
            token = strtok_r(NULL, DELIMITERS, &saveptr);
            if (token) {
                cmd->input_file = strdup(token);
            }
        } else if (strcmp(token, ">") == 0) {
            token = strtok_r(NULL, DELIMITERS, &saveptr);
            if (token) {
                cmd->output_file = strdup(token);
                cmd->append_mode = 0;
            }
        } else if (strcmp(token, ">>") == 0) {
            token = strtok_r(NULL, DELIMITERS, &saveptr);
            if (token) {
                cmd->output_file = strdup(token);
                cmd->append_mode = 1;
            }
        } else if (strcmp(token, "&") == 0) {
            cmd->background = 1;
        } else {
            
            if (cmd->arg_count < MAX_ARGS - 1) {
                cmd->args[cmd->arg_count++] = strdup(token);
            }
        }
        token = strtok_r(NULL, DELIMITERS, &saveptr);
    }

    cmd->args[cmd->arg_count] = NULL;
    free(line_copy);

    return cmd;
}


void free_command(Command *cmd) {
    if (cmd == NULL) return;

    for (int i = 0; i < cmd->arg_count; i++) {
        free(cmd->args[i]);
    }

    if (cmd->input_file) free(cmd->input_file);
    if (cmd->output_file) free(cmd->output_file);

    free(cmd);
}


int execute_command(Command *cmd) {
    pid_t pid;
    int status;
    int saved_stdin, saved_stdout;

    if (execute_builtin(cmd) == 0) {
        return 1;
    }

    saved_stdin = dup(STDIN_FILENO);
    saved_stdout = dup(STDOUT_FILENO);

    
    pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Ошибка создания процесса\n");
        return 1;
    } else if (pid == 0) {
        
        char env_parent[MAX_PATH_LENGTH + 16];
        snprintf(env_parent, sizeof(env_parent), "parent=%s", shell_path);
        putenv(env_parent);

        handle_redirection(cmd);

        if (execvp(cmd->args[0], cmd->args) == -1) {
            fprintf(stderr, "Ошибка выполнения команды: %s\n", cmd->args[0]);
            exit(EXIT_FAILURE);
        }
    } else {
        if (!cmd->background) {
            do {
                waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        } else {
            printf("[%d] %d\n", pid, pid);
        }
    }

    restore_redirection(saved_stdin, saved_stdout);

    return 1;
}


void handle_redirection(Command *cmd) {
    int fd;

    if (cmd->input_file != NULL) {
        fd = open(cmd->input_file, O_RDONLY);
        if (fd < 0) {
            fprintf(stderr, "Ошибка открытия файла для ввода: %s\n", cmd->input_file);
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }

    if (cmd->output_file != NULL) {
        if (cmd->append_mode) {
            fd = open(cmd->output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
        } else {
            fd = open(cmd->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        }

        if (fd < 0) {
            fprintf(stderr, "Ошибка открытия файла для вывода: %s\n", cmd->output_file);
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
}


void restore_redirection(int saved_stdin, int saved_stdout) {
    dup2(saved_stdin, STDIN_FILENO);
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdin);
    close(saved_stdout);
}


int execute_builtin(Command *cmd) {
    if (strcmp(cmd->args[0], "cd") == 0) {
        return cmd_cd(cmd);
    } else if (strcmp(cmd->args[0], "clr") == 0) {
        return cmd_clr(cmd);
    } else if (strcmp(cmd->args[0], "dir") == 0) {
        return cmd_dir(cmd);
    } else if (strcmp(cmd->args[0], "environ") == 0) {
        return cmd_environ(cmd);
    } else if (strcmp(cmd->args[0], "echo") == 0) {
        return cmd_echo(cmd);
    } else if (strcmp(cmd->args[0], "help") == 0) {
        return cmd_help(cmd);
    } else if (strcmp(cmd->args[0], "pause") == 0) {
        return cmd_pause(cmd);
    } else if (strcmp(cmd->args[0], "quit") == 0) {
        return cmd_quit(cmd);
    }

    return -1;
}


int cmd_cd(Command *cmd) {
    char *path;
    char cwd[MAX_PATH_LENGTH];

    if (cmd->arg_count == 1) {
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        }
        return 0;
    }

    path = cmd->args[1];

    if (chdir(path) != 0) {
        fprintf(stderr, "Ошибка: каталог '%s' не найден\n", path);
    } else {
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            setenv("PWD", cwd, 1);
        }
    }

    return 0;
}

int cmd_clr(Command *cmd) {
    printf("\033[2J\033[1;1H");
    fflush(stdout);
    return 0;
}

int cmd_dir(Command *cmd) {
    char *path;
    char command[MAX_LINE_LENGTH];

    if (cmd->arg_count > 1) {
        path = cmd->args[1];
    } else {
        path = ".";
    }

    snprintf(command, sizeof(command), "ls -la %s", path);
    system(command);

    return 0;
}

int cmd_environ(Command *cmd) {
    char **env = environ;

    while (*env != NULL) {
        printf("%s\n", *env);
        env++;
    }

    return 0;
}

int cmd_echo(Command *cmd) {
    for (int i = 1; i < cmd->arg_count; i++) {
        if (i > 1) printf(" ");
        printf("%s", cmd->args[i]);
    }
    printf("\n");

    return 0;
}

int cmd_help(Command *cmd) {
    FILE *help_file;
    char line[MAX_LINE_LENGTH];
    char help_path[MAX_PATH_LENGTH];

    help_file = fopen("readme", "r");

    if (help_file == NULL) {
        char *last_slash = strrchr(shell_path, '/');
        if (last_slash != NULL) {
            int len = last_slash - shell_path;
            strncpy(help_path, shell_path, len);
            help_path[len] = '\0';
            strcat(help_path, "/readme");
            help_file = fopen(help_path, "r");
        }
    }

    if (help_file == NULL) {
        fprintf(stderr, "Ошибка: файл справки readme не найден\n");
        return 0;
    }

    while (fgets(line, sizeof(line), help_file) != NULL) {
        printf("%s", line);
    }

    fclose(help_file);

    return 0;
}

int cmd_pause(Command *cmd) {
    printf("Нажмите клавишу Enter для продолжения...");
    fflush(stdout);

    while (getchar() != '\n') {
    }

    return 0;
}

int cmd_quit(Command *cmd) {
    exit(EXIT_SUCCESS);
    return 0;
}
