
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>

//INCLUDES for extra credit
//#include <signal.h>
//#include <pthread.h>
//-------------------------

#include "dshlib.h"
#include "rshlib.h"
#include "errno.h" //Used to handle system calls


/*
 * start_server(ifaces, port, is_threaded)
 *      ifaces:  a string in ip address format, indicating the interface
 *              where the server will bind.  In almost all cases it will
 *              be the default "0.0.0.0" which binds to all interfaces.
 *              note the constant RDSH_DEF_SVR_INTFACE in rshlib.h
 * 
 *      port:   The port the server will use.  Note the constant 
 *              RDSH_DEF_PORT which is 1234 in rshlib.h.  If you are using
 *              tux you may need to change this to your own default, or even
 *              better use the command line override -s implemented in dsh_cli.c
 *              For example ./dsh -s 0.0.0.0:5678 where 5678 is the new port  
 * 
 *      is_threded:  Used for extra credit to indicate the server should implement
 *                   per thread connections for clients  
 * 
 *      This function basically runs the server by: 
 *          1. Booting up the server
 *          2. Processing client requests until the client requests the
 *             server to stop by running the `stop-server` command
 *          3. Stopping the server. 
 * 
 *      This function is fully implemented for you and should not require
 *      any changes for basic functionality.  
 * 
 *      IF YOU IMPLEMENT THE MULTI-THREADED SERVER FOR EXTRA CREDIT YOU NEED
 *      TO DO SOMETHING WITH THE is_threaded ARGUMENT HOWEVER.  
 */
int start_server(char *ifaces, int port, int is_threaded){
    int svr_socket;
    int rc;

    //
    //TODO:  If you are implementing the extra credit, please add logic
    //       to keep track of is_threaded to handle this feature
    //

    svr_socket = boot_server(ifaces, port);
    if (svr_socket < 0){
        int err_code = svr_socket; 
        return err_code;
    }

    rc = process_cli_requests(svr_socket);

    stop_server(svr_socket);


    return rc;
}

/*
 * stop_server(svr_socket)
 *      svr_socket: The socket that was created in the boot_server()
 *                  function. 
 * 
 *      This function simply returns the value of close() when closing
 *      the socket.  
 */
int stop_server(int svr_socket){
    return close(svr_socket);
}

/*
 * boot_server(ifaces, port)
 *      ifaces & port:  see start_server for description.  They are passed
 *                      as is to this function.   
 * 
 *      This function "boots" the rsh server.  It is responsible for all
 *      socket operations prior to accepting client connections.  Specifically: 
 * 
 *      1. Create the server socket using the socket() function. 
 *      2. Calling bind to "bind" the server to the interface and port
 *      3. Calling listen to get the server ready to listen for connections.
 * 
 *      after creating the socket and prior to calling bind you might want to 
 *      include the following code:
 * 
 *      int enable=1;
 *      setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
 * 
 *      when doing development you often run into issues where you hold onto
 *      the port and then need to wait for linux to detect this issue and free
 *      the port up.  The code above tells linux to force allowing this process
 *      to use the specified port making your life a lot easier.
 * 
 *  Returns:
 * 
 *      server_socket:  Sockets are just file descriptors, if this function is
 *                      successful, it returns the server socket descriptor, 
 *                      which is just an integer.
 * 
 *      ERR_RDSH_COMMUNICATION:  This error code is returned if the socket(),
 *                               bind(), or listen() call fails. 
 * 
 */
int boot_server(char *ifaces, int port){
    int svr_socket = -1; // Initialize to -1
    int ret;
    struct sockaddr_in addr;
    int enable = 1;

    // TODO set up the socket - this is very similar to the demo code
    if ((svr_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        return ERR_RDSH_COMMUNICATION;
    }

    if (setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1) {
        perror("setsockopt failed");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    addr.sin_family = AF_INET;
    if (strcmp(ifaces, "0.0.0.0") == 0) {
        addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        if (inet_pton(AF_INET, ifaces, &addr.sin_addr) <= 0) {
            fprintf(stderr, "Invalid interface address: %s\n", ifaces);
            close(svr_socket);
            return ERR_RDSH_COMMUNICATION;
        }
    }
    addr.sin_port = htons(port);
    memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));

    if (bind(svr_socket, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind failed");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    /*
     * Prepare for accepting connections. The backlog size is set
     * to 20. So while one request is being processed other requests
     * can be waiting.
     */
    ret = listen(svr_socket, 20);
    if (ret == -1) {
        perror("listen");
        close(svr_socket); // Close the socket if listen fails
        return ERR_RDSH_COMMUNICATION;
    }

    printf("Server listening on %s:%d\n", ifaces, port);

    return svr_socket;
}

/*
 * process_cli_requests(svr_socket)
 *      svr_socket:  The server socket that was obtained from boot_server()
 *   
 *  This function handles managing client connections.  It does this using
 *  the following logic
 * 
 *      1.  Starts a while(1) loop:
 *  
 *          a. Calls accept() to wait for a client connection. Recall that 
 *             the accept() function returns another socket specifically
 *             bound to a client connection. 
 *          b. Calls exec_client_requests() to handle executing commands
 *             sent by the client. It will use the socket returned from
 *             accept().
 *          c. Loops back to the top (step 2) to accept connecting another
 *             client.  
 * 
 *          note that the exec_client_requests() return code should be
 *          negative if the client requested the server to stop by sending
 *          the `stop-server` command.  If this is the case step 2b breaks
 *          out of the while(1) loop. 
 * 
 *      2.  After we exit the loop, we need to cleanup.  Dont forget to 
 *          free the buffer you allocated in step #1.  Then call stop_server()
 *          to close the server socket. 
 * 
 *  Returns:
 * 
 *      OK_EXIT:  When the client sends the `stop-server` command this function
 *                should return OK_EXIT. 
 * 
 *      ERR_RDSH_COMMUNICATION:  This error code terminates the loop and is
 *                returned from this function in the case of the accept() 
 *                function failing. 
 * 
 *      OTHERS:   See exec_client_requests() for return codes.  Note that positive
 *                values will keep the loop running to accept additional client
 *                connections, and negative values terminate the server. 
 * 
 */
int process_cli_requests(int svr_socket){
    int     cli_socket = -1;
    int     rc = OK;

    struct sockaddr_in cli_addr;
    socklen_t cli_addr_len = sizeof(cli_addr);

    while(1){
        printf("No client connected\n");
        // TODO use the accept syscall to create cli_socket
        // and then exec_client_requests(cli_socket)

        // Check if server was unable to connect with a client, stops server from accepting more connections
        if ((cli_socket = accept(svr_socket, (struct sockaddr *)&cli_addr, &cli_addr_len)) == -1) {
            perror("accept failed");
            rc = ERR_RDSH_COMMUNICATION;
            break;
        }

        printf("Client connected");

        rc = exec_client_requests(cli_socket);
        close(cli_socket);

        if (rc == OK_EXIT) {
            printf("Client requested server to stop.\n");
            rc = OK_EXIT;
            break;
        } else if (rc == OK) {
            printf("Client exited.\n");
        } else if (rc == ERR_RDSH_COMMUNICATION) {
            perror("Communication error with client");
        }
    }

    stop_server(svr_socket);
    return rc;
}

/*
 * exec_client_requests(cli_socket)
 *      cli_socket:  The server-side socket that is connected to the client
 *   
 *  This function handles accepting remote client commands. The function will
 *  loop and continue to accept and execute client commands.  There are 2 ways
 *  that this ongoing loop accepting client commands ends:
 * 
 *      1.  When the client executes the `exit` command, this function returns
 *          to process_cli_requests() so that we can accept another client
 *          connection. 
 *      2.  When the client executes the `stop-server` command this function
 *          returns to process_cli_requests() with a return code of OK_EXIT
 *          indicating that the server should stop. 
 * 
 *  Note that this function largely follows the implementation of the
 *  exec_local_cmd_loop() function that you implemented in the last 
 *  shell program deliverable. The main difference is that the command will
 *  arrive over the recv() socket call rather than reading a string from the
 *  keyboard. 
 * 
 *  This function also must send the EOF character after a command is
 *  successfully executed to let the client know that the output from the
 *  command it sent is finished.  Use the send_message_eof() to accomplish 
 *  this. 
 * 
 *  Of final note, this function must allocate a buffer for storage to 
 *  store the data received by the client. For example:
 *     io_buff = malloc(RDSH_COMM_BUFF_SZ);
 *  And since it is allocating storage, it must also properly clean it up
 *  prior to exiting.
 * 
 *  Returns:
 * 
 *      OK:       The client sent the `exit` command.  Get ready to connect
 *                another client. 
 *      OK_EXIT:  The client sent `stop-server` command to terminate the server
 * 
 *      ERR_RDSH_COMMUNICATION:  A catch all for any socket() related send
 *                or receive errors. 
 */
int exec_client_requests(int cli_socket) {
    int io_size;
    command_list_t cmd_list;
    int rc = OK;
    int cmd_rc = 0;
    int last_rc = 0;
    char *io_buff;
    ssize_t bytes_received;
    pid_t child_pid;
    cmd_buff_t cmd;
    command_list_t clist;
    int parse_result;
    int wait_status;

    io_buff = malloc(RDSH_COMM_BUFF_SZ);
    if (io_buff == NULL){
        return ERR_RDSH_SERVER;
    }

    cmd._cmd_buffer = (char *)malloc(SH_CMD_MAX);
    if (cmd._cmd_buffer == NULL) {
        perror("Failed to allocate command buffer");
        free(io_buff);
        return ERR_RDSH_COMMUNICATION;
    }

    while(1) {
        // TODO use recv() syscall to get input
        bytes_received = recv(cli_socket, io_buff, RDSH_COMM_BUFF_SZ - 1, 0);
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                printf("Client disconnected.\n");
                rc = OK;
            } else {
                perror("Error receiving data from client");
                rc = ERR_RDSH_COMMUNICATION;
            }
            break;
        }
        io_buff[bytes_received] = '\0';
        printf("Server received: %s from client.\n", io_buff);

        // TODO build up a cmd_list
        if (strchr(io_buff, '|') != NULL) {
            parse_result = build_cmd_list(io_buff, &clist);
            if (parse_result == OK) {
                // TODO rsh_execute_pipeline to run your cmd_list
                int exit_code = rsh_execute_pipeline(cli_socket, &clist);
                cmd_rc = exit_code;
                if (exit_code != WARN_RDSH_NOT_IMPL) {
                    printf("Pipeline execution finished with exit code: %d\n", exit_code);
                } else {
                    fprintf(stderr, "rsh_execute_pipeline not implemented yet.\n");
                }
                // TODO send appropriate responses with send_message_string
                // - error constants for failures
                // - buffer contents from execute commands
                //  - etc.
                if (send_message_eof(cli_socket) != OK) {
                    perror("Error sending EOF after pipeline");
                }
            } else {
                fprintf(stderr, "Error parsing pipeline.\n");
                char *error_msg = "Error parsing pipeline.\n";
                send_message_string(cli_socket, error_msg);
                // TODO send_message_eof when done
                send_message_eof(cli_socket);
                cmd_rc = -1;
            }
        } else {
            strcpy(cmd._cmd_buffer, io_buff);
            parse_input(&cmd);
            printf("Server parsed command: '%s'\n", cmd.argv[0]);

            Built_In_Cmds result = rsh_built_in_cmd(&cmd);
            printf("rsh_built_in_cmd returned: %d\n", result);

            if (result == BI_CMD_EXIT) {
                printf("Client sent 'exit' command. Closing connection.\n");
                rc = OK;
                break;
            } else if (result == BI_CMD_STOP_SVR) {
                printf("Client sent 'stop-server' command. Shutting down server.\n");
                rc = OK_EXIT;
                break;
            } else if (result == BI_EXECUTED) {
                if (rsh_match_command(cmd.argv[0]) == BI_CMD_CD) {
                    char response[RDSH_COMM_BUFF_SZ];
                    if (cmd.argc > 1) {
                        snprintf(response, sizeof(response), "Successfully changed directory to %s\n", cmd.argv[1]);
                    } else {
                        snprintf(response, sizeof(response), "Changed directory to home.\n");
                    }
                    // TODO send appropriate responses with send_message_string
                    if (send_message_string(cli_socket, response) != OK) {
                        perror("Error sending cd response");
                    }
                    printf("Server sent back: %s (with EOF).\n", response);
                    // TODO send_message_eof when done
                    send_message_eof(cli_socket);
                    cmd_rc = 0;
                }
            } else if (result == BI_NOT_BI) {
                if (cmd.argv[0] != NULL) {
                    child_pid = fork();
                    if (child_pid == -1) {
                        perror("fork failed");
                        char *error_msg = "Error: Fork failed.\n";
                        // TODO send appropriate responses with send_message_string
                        send_message_string(cli_socket, error_msg);
                        // TODO send_message_eof when done
                        send_message_eof(cli_socket);
                        cmd_rc = -1;
                    } else if (child_pid == 0) {
                        dup2(cli_socket, STDOUT_FILENO);
                        dup2(cli_socket, STDERR_FILENO);
                        execvp(cmd.argv[0], cmd.argv);
                        perror("execvp failed");
                        exit(1);
                    } else {
                        waitpid(child_pid, &wait_status, 0);
                        if (WIFEXITED(wait_status)) {
                            cmd_rc = WEXITSTATUS(wait_status);
                        } else {
                            cmd_rc = -1;
                        }
                        // TODO send_message_eof when done
                        if (send_message_eof(cli_socket) != OK) {
                            perror("Error sending EOF after command");
                        }
                    }
                }
            }
        }
        last_rc = cmd_rc;
        printf("Command return code (cmd_rc): %d\n", cmd_rc);
        printf("Last return code (last_rc): %d\n", last_rc);
    }

    free(cmd._cmd_buffer);
    free(io_buff);

    return rc;
}
/*
 * send_message_eof(cli_socket)
 *      cli_socket:  The server-side socket that is connected to the client

 *  Sends the EOF character to the client to indicate that the server is
 *  finished executing the command that it sent. 
 * 
 *  Returns:
 * 
 *      OK:  The EOF character was sent successfully. 
 * 
 *      ERR_RDSH_COMMUNICATION:  The send() socket call returned an error or if
 *           we were unable to send the EOF character. 
 */
int send_message_eof(int cli_socket){
    int bytes_sent;

    //send one character, the EOF character.
    bytes_sent = send(cli_socket, &RDSH_EOF_CHAR, 1, 0);
    if (bytes_sent == 1){
        //this is what is expected, we sent a single character,
        //the EOF character, so we can return a good error code.
        //we use OK for this as defined in dshlib.h
        return OK;
    }

    //handle error and send back an appropriate error code
    //if bytes_sent < 0 that would indicate a network error
    //if it equals zero it indicates the character could not
    //be sent, which is also an error.  I could not imagine a
    //situation where bytes_sent > 1 since we told send to 
    //send exactly one byte, but if this happens it would also
    //be an error.

    //Ill just return a generic COMMUNICATION error we defined
    //for you in rshlib.h, but you can return different error
    //codes for different conditions if you want. 
    return ERR_RDSH_COMMUNICATION;
}

/*
 * send_message_string(cli_socket, char *buff)
 *      cli_socket:  The server-side socket that is connected to the client
 *      buff:        A C string (aka null terminated) of a message we want
 *                   to send to the client. 
 *   
 *  Sends a message to the client.  Note this command executes both a send()
 *  to send the message and a send_message_eof() to send the EOF character to
 *  the client to indicate command execution terminated. 
 * 
 *  Returns:
 * 
 *      OK:  The message in buff followed by the EOF character was 
 *           sent successfully. 
 * 
 *      ERR_RDSH_COMMUNICATION:  The send() socket call returned an error or if
 *           we were unable to send the message followed by the EOF character. 
 */
int send_message_string(int cli_socket, char *buff){
    int bytes_sent = send(cli_socket, buff, strlen(buff), 0);
    if (bytes_sent < 0) {
        perror("Error Sending Message");
        return ERR_RDSH_COMMUNICATION;
    }
    return send_message_eof(cli_socket);;
}


/*
 * rsh_execute_pipeline(int cli_sock, command_list_t *clist)
 *      cli_sock:    The server-side socket that is connected to the client
 *      clist:       The command_list_t structure that we implemented in
 *                   the last shell. 
 *   
 *  This function executes the command pipeline.  It should basically be a
 *  replica of the execute_pipeline() function from the last deliverable. 
 *  The only thing different is that you will be using the cli_sock as the
 *  main file descriptor on the first executable in the pipeline for STDIN,
 *  and the cli_sock for the file descriptor for STDOUT, and STDERR for the
 *  last executable in the pipeline.  See picture below:  
 * 
 *      
 *┌───────────┐                                                    ┌───────────┐
 *│ cli_sock  │                                                    │ cli_sock  │
 *└─────┬─────┘                                                    └────▲──▲───┘
 *      │   ┌──────────────┐     ┌──────────────┐     ┌──────────────┐  │  │    
 *      │   │   Process 1  │     │   Process 2  │     │   Process N  │  │  │    
 *      │   │              │     │              │     │              │  │  │    
 *      └───▶stdin   stdout├─┬──▶│stdin   stdout├─┬──▶│stdin   stdout├──┘  │    
 *          │              │ │   │              │ │   │              │     │    
 *          │        stderr├─┘   │        stderr├─┘   │        stderr├─────┘    
 *          └──────────────┘     └──────────────┘     └──────────────┘   
 *                                                      WEXITSTATUS()
 *                                                      of this last
 *                                                      process to get
 *                                                      the return code
 *                                                      for this function       
 * 
 *  Returns:
 * 
 *      EXIT_CODE:  This function returns the exit code of the last command
 *                  executed in the pipeline.  If only one command is executed
 *                  that value is returned.  Remember, use the WEXITSTATUS()
 *                  macro that we discussed during our fork/exec lecture to
 *                  get this value. 
 */
int rsh_execute_pipeline(int cli_sock, command_list_t *clist) {
    if (clist->num == 0) {
        return OK;
    }

    int num_commands = clist->num;
    int pipes[num_commands - 1][2];  // Array of pipes
    pid_t pids[num_commands];
    int  pids_st[num_commands];        // Array to store process IDs
    Built_In_Cmds bi_cmd;
    int exit_code;
    int i;

    // Create all necessary pipes
    for (i = 0; i < num_commands - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    for (i = 0; i < num_commands; i++) {
        // TODO this is basically the same as the piped fork/exec assignment, except for where you connect the begin and end of the pipeline (hint: cli_sock)
        pids[i] = fork();
        if (pids[i] == -1) {
            perror("fork failed");
            return ERR_RDSH_COMMUNICATION;
        }

        if (pids[i] == 0) { // Child process
            // TODO HINT you can dup2(cli_sock with STDIN_FILENO, STDOUT_FILENO, etc.

            if (i == 0) {
                if (dup2(cli_sock, STDIN_FILENO) == -1) {
                    perror("dup2 for stdin failed");
                    exit(1);
                }
            } else {
                if (dup2(pipes[i - 1][0], STDIN_FILENO) == -1) {
                    perror("dup2 for stdin failed");
                    exit(1);
                }
            }

            if (i == num_commands - 1) {
                if (dup2(cli_sock, STDOUT_FILENO) == -1) {
                    perror("dup2 for stdout failed");
                    exit(1);
                }
            } else {
                if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
                    perror("dup2 for stdout failed");
                    exit(1);
                }
            }

            if (dup2(cli_sock, STDERR_FILENO) == -1) {
                perror("dup2 for stderr failed");
                exit(1);
            }

            for (int i = 0; i < clist->num; i++) {
                close(pipes[i][0]);
                close(pipes[i][1]);
            }

            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            perror("execvp failed");
            exit(1);
        }
    }


    // Parent process: close all pipe ends
    for (int i = 0; i < clist->num; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Wait for all children
    for (int i = 0; i < clist->num; i++) {
        waitpid(pids[i], &pids_st[i], 0);
    }

    //by default get exit code of last process
    //use this as the return value
    exit_code = WEXITSTATUS(pids_st[clist->num - 1]);
    for (int i = 0; i < clist->num; i++) {
        //if any commands in the pipeline are EXIT_SC
        //return that to enable the caller to react
        if (WEXITSTATUS(pids_st[i]) == EXIT_SC)
            exit_code = EXIT_SC;
    }
    return exit_code;
}

/**************   OPTIONAL STUFF  ***************/
/****
 **** NOTE THAT THE FUNCTIONS BELOW ALIGN TO HOW WE CRAFTED THE SOLUTION
 **** TO SEE IF A COMMAND WAS BUILT IN OR NOT.  YOU CAN USE A DIFFERENT
 **** STRATEGY IF YOU WANT.  IF YOU CHOOSE TO DO SO PLEASE REMOVE THESE
 **** FUNCTIONS AND THE PROTOTYPES FROM rshlib.h
 **** 
 */

/*
 * rsh_match_command(const char *input)
 *      cli_socket:  The string command for a built-in command, e.g., dragon,
 *                   cd, exit-server
 *   
 *  This optional function accepts a command string as input and returns
 *  one of the enumerated values from the BuiltInCmds enum as output. For
 *  example:
 * 
 *      Input             Output
 *      exit              BI_CMD_EXIT
 *      dragon            BI_CMD_DRAGON
 * 
 *  This function is entirely optional to implement if you want to handle
 *  processing built-in commands differently in your implementation. 
 * 
 *  Returns:
 * 
 *      BI_CMD_*:   If the command is built-in returns one of the enumeration
 *                  options, for example "cd" returns BI_CMD_CD
 * 
 *      BI_NOT_BI:  If the command is not "built-in" the BI_NOT_BI value is
 *                  returned. 
 */
Built_In_Cmds rsh_match_command(const char *input)
{
    if (input == NULL) {
        return BI_NOT_BI;
    }
    if (strcmp(input, "exit") == 0) {
        return BI_CMD_EXIT;
    }
    if (strcmp(input, "stop-server") == 0) {
        return BI_CMD_STOP_SVR;
    }
    if (strcmp(input, "cd") == 0 || strncmp(input, "cd ", 3) == 0) {
        return BI_CMD_CD;
    }
    return BI_NOT_BI;
}

/*
 * rsh_built_in_cmd(cmd_buff_t *cmd)
 *      cmd:  The cmd_buff_t of the command, remember, this is the 
 *            parsed version fo the command
 *   
 *  This optional function accepts a parsed cmd and then checks to see if
 *  the cmd is built in or not.  It calls rsh_match_command to see if the 
 *  cmd is built in or not.  Note that rsh_match_command returns BI_NOT_BI
 *  if the command is not built in. If the command is built in this function
 *  uses a switch statement to handle execution if appropriate.   
 * 
 *  Again, using this function is entirely optional if you are using a different
 *  strategy to handle built-in commands.  
 * 
 *  Returns:
 * 
 *      BI_NOT_BI:   Indicates that the cmd provided as input is not built
 *                   in so it should be sent to your fork/exec logic
 *      BI_EXECUTED: Indicates that this function handled the direct execution
 *                   of the command and there is nothing else to do, consider
 *                   it executed.  For example the cmd of "cd" gets the value of
 *                   BI_CMD_CD from rsh_match_command().  It then makes the libc
 *                   call to chdir(cmd->argv[1]); and finally returns BI_EXECUTED
 *      BI_CMD_*     Indicates that a built-in command was matched and the caller
 *                   is responsible for executing it.  For example if this function
 *                   returns BI_CMD_STOP_SVR the caller of this function is
 *                   responsible for stopping the server.  If BI_CMD_EXIT is returned
 *                   the caller is responsible for closing the client connection.
 * 
 *   AGAIN - THIS IS TOTALLY OPTIONAL IF YOU HAVE OR WANT TO HANDLE BUILT-IN
 *   COMMANDS DIFFERENTLY. 
 */
Built_In_Cmds rsh_built_in_cmd(cmd_buff_t *cmd) {
    if (cmd == NULL || cmd->argv == NULL || cmd->argv[0] == NULL) {
        return BI_NOT_BI;
    }

    Built_In_Cmds matched_cmd = rsh_match_command(cmd->argv[0]);

    switch (matched_cmd) {
        case BI_CMD_EXIT:
            return BI_CMD_EXIT;
        case BI_CMD_STOP_SVR:
            return BI_CMD_STOP_SVR;
        case BI_CMD_CD:
            if (cmd->argc > 1) {
                chdir(cmd->argv[1]);
            } else {
                chdir(getenv("HOME"));
            }
            return BI_EXECUTED; 
        case BI_NOT_BI:
            return BI_NOT_BI;
        default:
            return BI_NOT_IMPLEMENTED;
    }
}