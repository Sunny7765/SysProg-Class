1. How does the remote client determine when a command's output is fully received from the server, and what techniques can be used to handle partial reads or ensure complete message transmission?

A remote client knows the output is fully received when it receives the end-of-stream marker from the server. Loops that repeatedly call recv until the marker is received could be used.

2. This week's lecture on TCP explains that it is a reliable stream protocol rather than a message-oriented one. Since TCP does not preserve message boundaries, how should a networked shell protocol define and detect the beginning and end of a command sent over a TCP connection? What challenges arise if this is not handled correctly?

The client sends commands that end with a null byte. Like our server uses RDSH_EOF_CHAR to be the end. If boundaries are not handled, the client might not display the full output or misread the next message. 

3. Describe the general differences between stateful and stateless protocols.

Stateful protocols keep information about past states between the client and server. Stateless protocols have each request treated as a separate and independent transaction. 

4. Our lecture this week stated that UDP is "unreliable". If that is the case, why would we ever use it?

Because UDP is fast and supports broadcasting and multicasting. And it is usable for application where a little data loss is ok at the application level like online gaming. 

5. What interface/abstraction is provided by the operating system to enable applications to use network communications?

Sockets API