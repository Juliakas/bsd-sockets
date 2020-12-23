# bsd-sockets
Server and client written in C, communicating through sockets.

When server is running, clients can connect to play a game of hangman.
Server accepts multiple clients with a help of process forking, however they play on separate game instances.

Compile each component separately using gcc. For example: `gcc -o client.o client/client.c` and `gcc -o server.o server/server.c`.

After compiling start the server `./server.o PORT_NUM`. For example `./server.o 6000`.
Then run the client `./client.o HOST PORT_NUM`. For example `./client.o localhost 6000`.
