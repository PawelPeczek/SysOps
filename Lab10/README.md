# C system programming
## CLIENT-SERVER APLLICATION with sockets
* Server allows workers (clients) to register themselves to serve as a worker in "cluster"
* User can send to Server tasks (mathematical operation such as +, -, *, /), which are being distributed over workers
* Workers are obligued to response for PING requests (KEEPALIVE) and do a computation on behalf of Server
* Sockets via AF_UNIX and IPv4 (both SOCK_STREAM -> ex1 AND SOCK_DGRAM -> ex2) used
* Server written using separate threads to listen to sockets, ping workers and listen to user requests

## ADDITIONAL FEATURES
* O(1) insert client by name thanks to hash map (I home hash function is decent)
* Redistribution of unfinished tasks of clients that don't respond for PING

## LIMITATIONS
* Redistribution is limited to one more try to send task to another worker -> no clever buffering implemented
* Redistribution would be more usefull provided that Clients could server theirs' services with more than one thread (now whole client is blocked by message from socket and long computation would cause connection refuse from Server because such Client stop to respond for PING) 

