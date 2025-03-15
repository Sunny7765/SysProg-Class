#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include "dshlib.h"


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

int last_return_code = 0;

int exec_local_cmd_loop() {
    cmd_buff_t cmd_buff;
    command_list_t clist;
    int parse_result;
    Built_In_Cmds bi_cmd;

    cmd_buff._cmd_buffer = (char *)malloc(SH_CMD_MAX);
    if (cmd_buff._cmd_buffer == NULL) {
        return ERR_MEMORY;
    }
    
    // TODO IMPLEMENT MAIN LOOP

    // TODO IMPLEMENT parsing input to cmd_buff_t *cmd_buff

    // TODO IMPLEMENT if built-in command, execute builtin logic for exit, cd (extra credit: dragon)
    // the cd command should chdir to the provided directory; if no directory is provided, do nothing

    // TODO IMPLEMENT if not built-in command, fork/exec as an external command
    // for example, if the user input is "ls -l", you would fork/exec the command "ls" with the arg "-l"
    
    cmd_buff.argc = 0;
    while (1) {
        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff._cmd_buffer, SH_CMD_MAX, stdin) == NULL) 
        {
            printf("\n");
            break;
        }

        cmd_buff._cmd_buffer[strcspn(cmd_buff._cmd_buffer, "\n")] = '\0';
        parse_result = build_cmd_list(cmd_buff._cmd_buffer, &clist);
        
        if (parse_result != OK) {
            if (parse_result == WARN_NO_CMDS) {
                printf(CMD_WARN_NO_CMD);
            } else if (parse_result == ERR_TOO_MANY_COMMANDS)
            {
                fprintf(stderr, CMD_ERR_PIPE_LIMIT, CMD_MAX);
            }
            continue;
        }

        if (clist.num == 0) {
            printf(CMD_WARN_NO_CMD);
            continue;
        }

        bi_cmd = match_command(clist.commands[0].argv[0]);

        if (bi_cmd != BI_NOT_BI) {
            if (bi_cmd == BI_CMD_EXIT) {
                printf("exiting...\n");
                break;
            } else if (bi_cmd == BI_CMD_CD) {
                if (clist.commands[0].argc == 1) {
                    chdir(getenv("HOME"));
                } else {
                    if (chdir(clist.commands[0].argv[1]) != 0) {
                        perror("chdir failed");
                        last_return_code = errno;
                    } else {
                        last_return_code = 0;
                    }
                }
            }
            else if (bi_cmd != BI_NOT_BI && bi_cmd != BI_CMD_EXIT && bi_cmd != BI_CMD_CD) {
                exec_built_in_cmd(&clist.commands[0]);
            }

        } else {
            last_return_code = execute_pipeline(&clist);
            if (last_return_code != OK) {
                printf(CMD_ERR_EXECUTE);
            }
        }
    }

    free(cmd_buff._cmd_buffer);
    return OK;
}

Built_In_Cmds match_command(const char *input) {
    if (input == NULL) return BI_NOT_BI;
    if (strcmp(input, EXIT_CMD) == 0) {
        return BI_CMD_EXIT;
    } else if (strcmp(input, "cd") == 0) {
        return BI_CMD_CD;
    }
    else {
        return BI_NOT_BI;
    }
}

Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {
    return BI_EXECUTED; 
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
    size_t len = strlen(trimmed_line);
    while (len > 0 && trimmed_line[len - 1] == ' ') {
        trimmed_line[--len] = '\0';
    }


    clist->num = 0;
    char *command_segment;
    char *remaining_input = trimmed_line;

    while ((command_segment = strsep(&remaining_input, PIPE_STRING)) != NULL) {
        if (strlen(command_segment) == 0) {
            continue;
        }
        
        if (clist->num >= CMD_MAX) {
            fprintf(stderr, CMD_ERR_PIPE_LIMIT, CMD_MAX);
            free(original_line);
            return ERR_TOO_MANY_COMMANDS;
        }

        cmd_buff_t *current_cmd_buff = &(clist->commands[clist->num]);

        current_cmd_buff->_cmd_buffer = (char *)malloc(SH_CMD_MAX);
        if (current_cmd_buff->_cmd_buffer == NULL) {
            free(original_line); 
            return ERR_MEMORY;
        }
        memset(current_cmd_buff->_cmd_buffer, 0, SH_CMD_MAX);

        strcpy(current_cmd_buff->_cmd_buffer, command_segment);

        parse_input(current_cmd_buff);

        clist->num++;
    }

    free(original_line); 
    return OK;
}

//Refactored from Professor Brian Mitchell's Demos
int execute_pipeline(command_list_t *clist) {
    if (clist->num == 0) {
        return OK;
    }

    int pipes[CMD_MAX - 1][2];  // Array of pipes
    pid_t pids[CMD_MAX];        // Array to store process IDs

    // Create all necessary pipes
    for (int i = 0; i < clist->num - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return ERR_EXEC_CMD;
        }
    }

    // Create processes for each command
    for (int i = 0; i < clist->num; i++) {
        pids[i] = fork();
        if (pids[i] == -1) {
            perror("fork");
            return ERR_EXEC_CMD;
        }

        if (pids[i] == 0) {  // Child process
            // Set up input pipe for all except first process
            if (i > 0) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }

            // Set up output pipe for all except last process
            if (i < clist->num - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            // Close all pipe ends in child
            for (int j = 0; j < clist->num - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // Execute command
            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            perror("execvp");
            exit(ERR_EXEC_CMD);
        }
    }

    // Parent process: close all pipe ends
    for (int i = 0; i < clist->num - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Wait for all children
    int status;
    int last_command_exit_status = OK;
    for (int i = 0; i < clist->num; i++) {
        if (waitpid(pids[i], &status, 0) == -1) {
            perror("waitpid");
            last_command_exit_status = ERR_EXEC_CMD;
        } else {
            if (WIFEXITED(status)) {
                last_command_exit_status = WEXITSTATUS(status);
            }
        }
    }
    return last_command_exit_status;
}