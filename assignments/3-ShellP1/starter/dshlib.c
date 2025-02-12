#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dshlib.h"

/*
 *  build_cmd_list
 *    cmd_line:     the command line from the user
 *    clist *:      pointer to clist structure to be populated
 *
 *  This function builds the command_list_t structure passed by the caller
 *  It does this by first splitting the cmd_line into commands by spltting
 *  the string based on any pipe characters '|'.  It then traverses each
 *  command.  For each command (a substring of cmd_line), it then parses
 *  that command by taking the first token as the executable name, and
 *  then the remaining tokens as the arguments.
 *
 *  NOTE your implementation should be able to handle properly removing
 *  leading and trailing spaces!
 *
 *  errors returned:
 *
 *    OK:                      No Error
 *    ERR_TOO_MANY_COMMANDS:   There is a limit of CMD_MAX (see dshlib.h)
 *                             commands.
 *    ERR_CMD_OR_ARGS_TOO_BIG: One of the commands provided by the user
 *                             was larger than allowed, either the
 *                             executable name, or the arg string.
 *
 *  Standard Library Functions You Might Want To Consider Using
 *      memset(), strcmp(), strcpy(), strtok(), strlen(), strchr()
 */


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

    int command_count = 0;
    char *saveptr1, *saveptr2; 
    char *command = strtok_r(trimmed_line, PIPE_STRING, &saveptr1); 

    while (command != NULL && command_count < CMD_MAX) { 
        memset(&clist->commands[command_count], 0, sizeof(command_t));

        char *cmd = strtok_r(command, " ", &saveptr2);
        if (cmd != NULL) {
            strncpy(clist->commands[command_count].exe, cmd, EXE_MAX - 1);
        }

        char *arg = strtok_r(NULL, " ", &saveptr2);
        while (arg != NULL) {
            if (strlen(clist->commands[command_count].args) + strlen(arg) + 2 >= ARG_MAX) {
                free(original_line);
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }

            if (clist->commands[command_count].args[0] != '\0') {
                strcat(clist->commands[command_count].args, " ");
            }
            strcat(clist->commands[command_count].args, arg);

            arg = strtok_r(NULL, " ", &saveptr2);
        }

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

