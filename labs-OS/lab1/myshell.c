#include "myshell.h"

char shell_path[MAX_PATH_LENGTH];



int main(int argc, char *argv[]) {
    FILE *input;


    if (realpath(argv[0], shell_path) == NULL) {
        strcpy(shell_path, argv[0]);
    }


    char env_shell[MAX_PATH_LENGTH + 16];
    snprintf(env_shell, sizeof(env_shell), "shell=%s", shell_path);
    putenv(env_shell);


    if (argc == 2) {

        input = fopen(argv[1], "r");
        if (input == NULL) {
            fprintf(stderr, "Ошибка: не удалось открыть файл '%s'\n", argv[1]);
            exit(EXIT_FAILURE);
        }
    } else if (argc == 1) {

        input = stdin;
    } else {
        fprintf(stderr, "Использование: %s [batchfile]\n", argv[0]);
        exit(EXIT_FAILURE);
    }


    shell_loop(input);

    if (input != stdin) {
        fclose(input);
    }

    return EXIT_SUCCESS;
}


void shell_loop(FILE *input) {
    char *line;
    Command *cmd;
    int status = 1;

    do {

        if (input == stdin) {
            show_cwd();
        }


        line = read_line(input);


        if (line == NULL) {
            printf("\n");
            break;
        }


        if (strlen(line) == 0) {
            free(line);
            continue;
        }


        cmd = parse_line(line);

        if (cmd != NULL && cmd->arg_count > 0) {

            status = execute_command(cmd);
        }


        free_command(cmd);
        free(line);

    } while (status);
}
