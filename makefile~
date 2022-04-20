all: client server

server: server.o nom.h
	gcc -o server server.o

server.o: server.c nom.h
	gcc -c server.c

client: client.o nom.h
	gcc -o client client.o

client.o: client.c nom.h
	gcc -c client.c
