1. In this assignment I suggested you use `fgets()` to get user input in the main while loop. Why is `fgets()` a good choice for this application?

    > **Answer**:  Yes, becuase it can read the input and be able to check if it has exceeded the buffer size or not. It also helps with formatting because we can detect newlines and other empty spaces.

2. You needed to use `malloc()` to allocte memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?

    > **Answer**: Malloc allows for dynamic memory allocation so that the size of the input can vary. If it was a fixed-size array, it may lead to not enough memory for the required command or too much memory being allocated.


3. In `dshlib.c`, the function `build_cmd_list(`)` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

    > **Answer**: If spaces are not trimmed, it would use unnecessary memory. The shell might also misinterpret the command because of parsing issues.

4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google _"linux shell stdin stdout stderr explained"_ to get started.

- One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

    > **Answer**:  
    Redirecting stdout and stderr separately: > redirects stdout to output.txt and 2> redirects stderr to error.txt. It will be challenging to parse the redirection symbols and separate file descriptors for each symbol

    Redirecting stdin from a file: stdin would read inputs from a txt file. It will be challenging to detect the < symbol, get the file descriptors, open the file, reading, and the handling errors.

    stdout appends instead of overwriting: >> would append the output to a txt file instead of overwriting it. It would be challenging to handle creating a new file/finding a file with the name, opening the file, and then appending it. 



- You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

    > **Answer**:  Redirection involve the inputs and outputs of files. Piping is passing of outputs from one command to another command as an input.

- STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

    > **Answer**:  It lets different results to be correctly identified so that debugging and finding the results is easy.

- How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

    > **Answer**:  We can display them separately to avoid confusion. But in cases where we would want to merge them, we would just merge them with redirection: 2>&1 which redirects STDERR to STDOUT