1. Your shell forks multiple child processes when executing piped commands. How does your implementation ensure that all child processes complete before the shell continues accepting user input? What would happen if you forgot to call waitpid() on all child processes?

After I set up all the required pipes, the parent process closes all the pipe ends and calls waitpid(). If I forgot to call waitpid, it would create a zombie process where it could lead to memory issues. 

2. The dup2() function is used to redirect input and output file descriptors. Explain why it is necessary to close unused pipe ends after calling dup2(). What could go wrong if you leave pipes open?

Because the file descriptors still use system resources which can cause resource issues. It could also cause deadlocks when processes don't receive the EOF signals involved in the pipeline. 

3. Your shell recognizes built-in commands (cd, exit, dragon). Unlike external commands, built-in commands do not require execvp(). Why is cd implemented as a built-in rather than an external command? What challenges would arise if cd were implemented as an external process?

Because built in commands run in the shell process itself, which lets cd change the shell's working directory. If it were to be implemented externally, it wouldn't be able to change the directory of the parent shell. 

4. Currently, your shell supports a fixed number of piped commands (CMD_MAX). How would you modify your implementation to allow an arbitrary number of piped commands while still handling memory allocation efficiently? What trade-offs would you need to consider?

I would probably use dynamic memory allocation like malloc and use dynamically allocated arrays. It would have the trade-off of needing more memory and may affect performance. 
