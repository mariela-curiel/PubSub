all: publicador suscriptor central

central: central.o nom.h
	gcc -o central central.o -pthread

central.o: central.c nom.h
	gcc -c central.c -pthread

publicador: publicador.o nom.h
	gcc -o publicador publicador.o

publicador.o: publicador.c nom.h
	gcc -c publicador.c

suscriptor: suscriptor.o nom.h
	gcc -o suscriptor suscriptor.o

suscriptor.o: suscriptor.c nom.h
	gcc -c suscriptor.c
