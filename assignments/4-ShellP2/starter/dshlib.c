#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
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

int exec_local_cmd_loop()
{
    cmd_buff_t cmd_buff;

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
    

    while (1) {
        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff._cmd_buffer, SH_CMD_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }

        cmd_buff._cmd_buffer[strcspn(cmd_buff._cmd_buffer, "\n")] = '\0';
        parse_input(&cmd_buff);
        
        if (cmd_buff.argc == 0) {
            printf(CMD_WARN_NO_CMD);
            continue;
        }

        if (strcmp(cmd_buff.argv[0], EXIT_CMD) == 0) {
            break;
        }

        if (strcmp(cmd_buff.argv[0], "cd") == 0) {
            if (cmd_buff.argc == 1) {
                chdir(getenv("HOME"));
            } else {
                if (chdir(cmd_buff.argv[1]) != 0) {
                    perror("chdir failed");
                }
            }
            continue;
        }

        pid_t pid = fork();
        if (pid == 0) {
           execvp(cmd_buff.argv[0], cmd_buff.argv);
           perror("exec failure");
           exit(1);
        } else {
            wait(NULL);
        }
    }

    free(cmd_buff._cmd_buffer);
    return OK;
}
