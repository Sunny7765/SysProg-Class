#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "dshlib.h"

/**** 
 **** FOR REMOTE SHELL USE YOUR SOLUTION FROM SHELL PART 3 HERE
 **** THE MAIN FUNCTION CALLS THIS ONE AS ITS ENTRY POINT TO
 **** EXECUTE THE SHELL LOCALLY
 ****
 */

/*
 * Implement your exec_local_cmd_loop function by building a loop that prompts the 
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.
 * 
 *      while(1){
 *        printf("%s", SH_PROMPT);
 *        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
 *           printf("\n");
 *           break;
 *        }
 *        //remove the trailing \n from cmd_buff
 *        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
 * 
 *        //IMPLEMENT THE REST OF THE REQUIREMENTS
 *      }
 * 
 *   Also, use the constants in the dshlib.h in this code.  
 *      SH_CMD_MAX              maximum buffer size for user input
 *      EXIT_CMD                constant that terminates the dsh program
 *      SH_PROMPT               the shell prompt
 *      OK                      the command was parsed properly
 *      WARN_NO_CMDS            the user command was empty
 *      ERR_TOO_MANY_COMMANDS   too many pipes used
 *      ERR_MEMORY              dynamic memory management failure
 * 
 *   errors returned
 *      OK                     No error
 *      ERR_MEMORY             Dynamic memory management failure
 *      WARN_NO_CMDS           No commands parsed
 *      ERR_TOO_MANY_COMMANDS  too many pipes used
 *   
 *   console messages
 *      CMD_WARN_NO_CMD        print on WARN_NO_CMDS
 *      CMD_ERR_PIPE_LIMIT     print on ERR_TOO_MANY_COMMANDS
 *      CMD_ERR_EXECUTE        print on execution failure of external command
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 1+)
 *      malloc(), free(), strlen(), fgets(), strcspn(), printf()
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 2+)
 *      fork(), execvp(), exit(), chdir()
 */
void parse_input(cmd_buff_t *cmd_buff) {
    cmd_buff->argc = 0;
    char *input = cmd_buff->_cmd_buffer;
    bool in_word = false;
    
    while (*input) {
        while (isspace(*input)) {
            *input++ = '\0';
        }
        
        if (*input == '\0') {
            break;
        }

        if (*input == '"') {
            in_word = true;
            input++;
        }

        cmd_buff->argv[cmd_buff->argc++] = input;

        while (*input) {
            if (in_word) {
                if (*input == '"') {
                    *input = '\0';
                    input++;
                    break;
                }
            } else {
                if (isspace(*input)) {
                    *input = '\0';
                    input++;
                    break;
                }
            }
            input++;
        }
        in_word = false;
    }
    cmd_buff->argv[cmd_buff->argc] = NULL;
}

int build_cmd_list(char *cmd_line, command_list_t *clist) {
    if (cmd_line == NULL || strlen(cmd_line) == 0) {
        printf(CMD_WARN_NO_CMD);
        return WARN_NO_CMDS;
    }

    char *original_line = strdup(cmd_line);
    if (original_line == NULL) {
        return ERR_CMD_OR_ARGS_TOO_BIG;
    }

    char *trimmed_line = original_line;
    while (*trimmed_line == ' ') trimmed_line++; // 

    size_t len = strlen(trimmed_line);
    while (len > 0 && trimmed_line[len - 1] == ' ') {
        trimmed_line[--len] = '\0'; 
    }

    if (*trimmed_line == '\0') {
        printf(CMD_WARN_NO_CMD);
        free(original_line);
        return WARN_NO_CMDS;
    }

    int command_count = 0;
    char *saveptr1, *saveptr2;
    char *command = strtok_r(trimmed_line, PIPE_STRING, &saveptr1);

    while (command != NULL && command_count < CMD_MAX) {
        cmd_buff_t *cmd_buff = &clist->commands[command_count];
        memset(cmd_buff, 0, sizeof(cmd_buff_t));

        int arg_index = 0;
        char *arg = strtok_r(command, " ", &saveptr2);

        while (arg != NULL && arg_index < CMD_ARGV_MAX - 1) {
            cmd_buff->argv[arg_index] = strdup(arg);
            if (cmd_buff->argv[arg_index] == NULL) {
                free(original_line);
                return ERR_CMD_OR_ARGS_TOO_BIG; 
            }
            arg_index++;
            arg = strtok_r(NULL, " ", &saveptr2);
        }

        cmd_buff->argv[arg_index] = NULL;
        cmd_buff->argc = arg_index;

        command_count++;
        command = strtok_r(NULL, PIPE_STRING, &saveptr1);
    }

    if (command != NULL) {
        free(original_line);
        return ERR_TOO_MANY_COMMANDS;
    }

    clist->num = command_count;
    free(original_line);
    return OK;
}

int execute_cmd_list(command_list_t *clist) {
    if (clist->num == 0) {
        return WARN_NO_CMDS;
    }

    int last_return_code = 0;  // Track return code for command execution

    // Handle single command execution
    if (clist->num == 1) {
        cmd_buff_t cmd_buff;
        cmd_buff._cmd_buffer = (char *)malloc(SH_CMD_MAX);
        if (cmd_buff._cmd_buffer == NULL) {
            return ERR_MEMORY;
        }

        // Prepare command buffer with executable and arguments
        snprintf(cmd_buff._cmd_buffer, SH_CMD_MAX, "%s %s",
                 clist->commands[0].exe, clist->commands[0].args);

        parse_input(&cmd_buff);

        if (cmd_buff.argc == 0) {
            free(cmd_buff._cmd_buffer);
            return WARN_NO_CMDS;
        }

        // Handle built-in "cd" command
        if (strcmp(cmd_buff.argv[0], "cd") == 0) {
            if (cmd_buff.argc == 1) {
                chdir(getenv("HOME"));
            } else {
                if (chdir(cmd_buff.argv[1]) != 0) {
                    perror("chdir failed");
                    last_return_code = errno;
                } else {
                    last_return_code = 0;
                }
            }
            free(cmd_buff._cmd_buffer);
            return OK;
        }

        // Fork and execute external command
        pid_t pid = fork();
        if (pid == 0) {
            execvp(cmd_buff.argv[0], cmd_buff.argv);
            perror("exec failure");
            exit(errno);
        } else {
            int status;
            wait(&status);
            if (WIFEXITED(status)) {
                last_return_code = WEXITSTATUS(status);
                if (last_return_code != 0) {
                    printf(CMD_ERR_EXECUTE);
                }
            }
        }
        free(cmd_buff._cmd_buffer);
        return OK;
    } else {
        // Handle multiple commands with pipes
        int pipes[clist->num - 1][2];

        // Create pipes for command chaining
        for (int i = 0; i < clist->num - 1; i++) {
            if (pipe(pipes[i]) == -1) {
                perror("pipe");
                return ERR_MEMORY;
            }
        }

        for (int i = 0; i < clist->num; i++) {
            pid_t pid = fork();
            if (pid == 0) {  // Child process
                // Set up stdin/stdout for pipe redirection
                if (i > 0) {
                    dup2(pipes[i - 1][0], STDIN_FILENO);  // Read from previous pipe
                }
                if (i < clist->num - 1) {
                    dup2(pipes[i][1], STDOUT_FILENO);  // Write to current pipe
                }

                // Close all pipes in child process
                for (int j = 0; j < clist->num - 1; j++) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }

                // Prepare command buffer
                cmd_buff_t cmd_buff;
                cmd_buff._cmd_buffer = (char *)malloc(SH_CMD_MAX);
                if (cmd_buff._cmd_buffer == NULL) {
                    exit(ERR_MEMORY);
                }

                snprintf(cmd_buff._cmd_buffer, SH_CMD_MAX, "%s %s",
                         clist->commands[i].exe, clist->commands[i].args);

                parse_input(&cmd_buff);

                // Execute the command
                execvp(cmd_buff.argv[0], cmd_buff.argv);
                perror("exec failure");
                exit(errno);
            } else if (pid < 0) {
                perror("fork");
                return ERR_MEMORY;
            }
        }

        // Parent process: close pipes and wait for all children
        for (int i = 0; i < clist->num - 1; i++) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }

        // Wait for child processes to finish
        int status;
        for (int i = 0; i < clist->num; i++) {
            wait(&status);
            if (WIFEXITED(status)) {
                if (WEXITSTATUS(status) != 0) {
                    printf(CMD_ERR_EXECUTE);
                }
            }
        }
    }

    return OK;
}

int exec_local_cmd_loop() {
    char cmd_buff[SH_CMD_MAX];
command_list_t clist;
    int rc;

    while (1) {
        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }

        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        char *trimmed_cmd = cmd_buff;
        while (*trimmed_cmd == ' ') trimmed_cmd++;
        size_t len = strlen(trimmed_cmd);
        while (len > 0 && trimmed_cmd[len - 1] == ' ') {
            trimmed_cmd[--len] = '\0';
        }

        if (*trimmed_cmd == '\0') {
            printf(CMD_WARN_NO_CMD);
            continue;
        }

        if (strcmp(trimmed_cmd, EXIT_CMD) == 0) {
            break;
        }

        rc = build_cmd_list(trimmed_cmd, &clist);

        switch (rc) {
            case OK:
                rc = execute_cmd_list(&clist);
                if (rc != OK) {
                    printf(CMD_ERR_EXECUTE);
                }
                break;
            case WARN_NO_CMDS:
                printf(CMD_WARN_NO_CMD);
                break;
            case ERR_TOO_MANY_COMMANDS:
                printf(CMD_ERR_PIPE_LIMIT);
                break;
            case ERR_MEMORY:
                printf("Memory allocation error\n");
                break;
            default:
                printf("Unknown error\n");
                break;
        }
    }

    return OK;
}