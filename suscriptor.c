/* 
Autor: Mariela Curiel
funcion: Suscriptor que envia uno de los dos topicos. Las llamadas al sistema no están validadas adecuadamente. 
Se invoca con el nombre del pipe que lo comunica al proceso central.c  y el topico (1 ó 2).  
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "nom.h"



int fd1;
char mensaje[TAMNEW];

int main (int argc, char **argv)
{
  int  fd, pid, creado = 0, cuantos=0;
  datap datos;
  char nombre[TAMNOMBRE];
 
  
  mode_t fifo_mode = S_IRUSR | S_IWUSR;
 
  
  // Se abre el pipe cuyo nombre se recibe como argumento del main. 
  do { 
     fd = open(argv[1], O_WRONLY);
     if (fd == -1) {
         perror("Suscriptor pipe");
         printf(" Se volvera a intentar despues\n");
	 sleep(5);        
     } else creado = 1;
  } while (creado == 0);


  datos.pid = getpid();
  datos.topico = atoi(argv[2]);
  
  // Nombre de un segundo pipe
  sprintf(nombre, "receptor%d", (int) getpid());
  //printf("Nombre = %s\n", nombre);
  strcpy(datos.segundopipe, nombre);
  //strcpy(datos.segundopipe, "mypipe");
  // Se crea un segundo pipe para la comunicacion con el server.
 
  if (mkfifo (datos.segundopipe, fifo_mode) == -1) {
     perror("Client  mkfifo");
     exit(1);
  }
  // se envia el nombre del pipe al otro proceso. 
   write(fd, &datos , sizeof(datos));

   // Se abre el segundo pipe
  creado = 0;
  do { 
     if ((fd1 = open(datos.segundopipe, O_RDONLY)) == -1) {
        perror(" Cliente  Abriendo el segundo pipe. Se volvera a intentar ");
        sleep(5);
     } else creado = 1; 
   } while (creado == 0);
 
  
   // Se leen varias noticias por el segundo pipe

	
  do {	
      cuantos = read(fd1, mensaje, TAMNEW);
      if (cuantos <= 0) break;
      printf("El proceso lee la noticia  %s \n", mensaje);

  } while (cuantos > 0);
 
  close(fd);
  close(fd1);
  unlink(nombre); 
  exit(0);

  
}

