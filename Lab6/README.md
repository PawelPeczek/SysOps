# C system programming
## Message queues
* SVR4 message queues client-server communication
* and the same example app with POSIX standard

## Explanation of communication protocol
When a client wants to establish connection with server it opens server-public queue where all requests from clienst are send. Then the client sends initial request with key/name its private queue that will be used by server to send back responses. Server receives the init request and assigns client ID-number for a client which is send back. After that a client is able to send four types of requests using its ID:
* MIRRR txt
* TIME
* CALC type op1 op2
where type can be:
  * ADD
  * MUL 
  * SUB
  * DIV
  * END - which cause the server to stop. Before that the server processes all requests that was sent to the END-request timestamp. Having processed all pending requests the server sends CL_END requests to clents (which cause the client could close its queue and (in POSIX) close the server-queue). Having received CL_END, the client send back CL_ACK so that all communication channels can be closed properly before returning control to the system.

## Client usage
In client communictaion loop, an user is able to use all types of requests discussed above and two special commands:
* QUIT
* FILE filename - which enables sending requests that are saved in batch-file. The syntax of batch file is the same, but it's not recommended to use FILE and QUIT command.
