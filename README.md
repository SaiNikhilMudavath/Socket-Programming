Part-1
This part is client-server communication using TCP sockets. 
The goal of the client program is to read a list of words from a file located on the server and count the frequency of the words.

Part-2
In this part, server is able to handle multiple concurrent client connections. 
The server is listening on the same port for all connections(TCP Multiplexing).

Part-3:
The server is grumpy in this case. It can only serve a client at a time. 
If the server receives a new request while serving an existing request, the server halts both requests and indicates them to come after some time.
The existing client is also expected to discard any communication it received during that specific request.
This part implements communication protocols like Slotted Aloha, Binary Exponential Backoff (BEB), Sensing and BEB.

Part-4:
Server can maintain multiple concurrent active connections. 
However, this time, the server schedules concurrent data requests using a centralized scheduling algorithm.
The scheduling policies are FIFO, Fair Scheduling.
