1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > **Answer**: Because fork creates a new process, which lets the shell perform actions before and after the command execution. Just using execvp would not create a new process and not be able to run the lines that we have written.

2. What happens if the fork() system call fails? How does your implementation handle this scenario?

    > **Answer**:  It means that fork() did not create a new. My implementation exits the program.

3. How does execvp() find the command to execute? What system environment variable plays a role in this process?

    > **Answer**:  It finds the path environment variable to find the command.

4. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**:  It is to stop the new processes from becoming a zombie process where the resources inside it cannot be accessed. If wait is not called, the process will stay in the system and use unnecessary resources. 

5. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

    > **Answer**:  It extracts the exit status of a new process which can help us determine if it ran correctly.

6. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

    > **Answer**:  My implementation handles it by handling quoted arguments as a single argument so that the spaces within are preserved. This is necessary because it allows quoted arguments to be treated as a single unit as it would be registered as multiple otherwise.

7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**:  Yes, it was really challenging trying to just switch it from accepting a command line to using cmd_buff instead. But I had to figure our how to not parse the spaces in the middle of the argument like for hello,       world. 

8. For this quesiton, you need to do some research on Linux signals. You can use [this google search](https://www.google.com/search?q=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&oq=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBBzc2MGowajeoAgCwAgA&sourceid=chrome&ie=UTF-8) to get started.

- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**: Signals communicate process events between the kernel and processes. Unlike IPCs, signals are used for controlling processes and not for communication.

- Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

    > **Answer**:  SIGKILL forcefully terminates a process. SIGTERM gives the process an opportunity to cleanup operations before exiting. SIGINT interrupts the running process.

- What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

    > **Answer**:  The process pauses until it receives a continue signal. It can't be ignored because it is used for debugging and process control. 
