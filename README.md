# Socket Programming
There is a txt file with server which has words seperated by commas. The goal is to send this file to a client which requests for this file.

##
Each part has a config.json, which specifies the server's ip and port which is requried for client to initiate the connection. It also has an attribute (input_file) for location of the file in the server. It also has attribute k which is the number of words the server responds with given a offset by client and p, the number of words sent in a packet by server to client. It has num_clients attribute which specifies the number of clients to simulate using pthreads.


## Part-1
This part is client-server communication using TCP sockets. 
The goal of the client program is to read a list of words from a file located on the server and count the frequency of the words.

To run, update the attributes in config.json, then compile client.cpp and server.cpp. Run the executables. This produces a output.txt file at client which has words and their frequencies.

## Part-2
In this part, server is able to handle multiple concurrent client connections. 
The server is listening on the same port for all connections(TCP Multiplexing).

To run, update the attributes in config.json, then compile client.cpp and server.cpp. Run the executables. This produces output{i}.txt files (i represents the i'th client; 0 ≤ i < num_clients) which has words and their frequencies.

## Part-3:
The server is grumpy in this case. It can only serve a client at a time. 
If the server receives a new request while serving an existing request, the server halts both requests and indicates them to come after some time.
The existing client is also expected to discard any communication it received during that specific request.
This part implements communication protocols like Slotted Aloha, Binary Exponential Backoff (BEB), Sensing and BEB.

To run, update the attributes in config.json, then compile client_slotted_aloha.cpp, client_beb.cpp, client_sense.cpp, server_aloha.cpp, server_sense.cpp. Let the executables be  client_slotted_aloha, client_beb, client_sense, server_aloha, server_sense. To simulate the client-server communication protocol
1. Slotted Aloha, run the executables client_slotted_aloha, server_aloha.
2. BEB, run the executables client_beb, server_aloha.
3. Sensing and BEB, run the executables client_sense, server_sense.

This produces output{i}.txt files (i represents the i'th client; 0 ≤ i < num_clients) which has words and their frequencies.

## Part-4:
Server can maintain multiple concurrent active connections. 
However, this time, the server schedules concurrent data requests using a centralized scheduling algorithm.
The scheduling policies are FIFO, Fair Scheduling.

To run, update the attributes in config.json, then compile client.cpp, fair_schedule.cpp, fifo.cpp. The executables are client, fair_schedule, fifo respectively. To simulate server scheduling algo
1. fifo, run the executables client, fifo
2. Round Robin/ Fair Scheduling, run the executables client, fair_schedule.

##
There are also script files for each part which run the client-server simulation with various values of attributes k,p and log and plot the average completion times for a client.
To generate plots or compare the completion times, execute the following commands
1. make build, to compile the cpp files
2. make run, produces times.txt file, which has the average completion time for a client
3. make plot, generates a plot with average completion times for each communication protocol or various values of attributes.

The results can be found in report.pdf
