/* 
Autor: Mariela Curiel
funcion: Publicador que envia noticias asociadas a dos topicos. El nombre del pipe que lo comunica con  proceso central.c se recibe como argumento del main. 

No está validadas todas las llamadas al sistema. Este publicador envía 10 noticias. 
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "nom.h"


int main (int argc, char **argv)
{
  int  fd, fd1, n, cuantos,res,creado=0, topico;
  datap datos;
  newp  noti;
  char *mensajes1[]={"Biologia", "La vegetacion es exuberante", "Animales", "Zorro", "El Loro"};
  char *mensajes2[]={"Petro", "pais en elecciones", "Fico en Medellin", "Fajardo quedo de tercero", "Pronto Elegimos"};    
    
  


     // Se abre el pipe cuyo nombre se recibe como argumento del main. 
  do { 
     fd = open(argv[1], O_WRONLY);
     if (fd == -1) {
         perror("Publicador pipe");
         printf(" Se volvera a intentar despues\n");
	 sleep(5);        
     } else creado = 1;
  } while (creado == 0);


     
   for (int i = 0; i < NNOTICIAS; i++) {
      noti.topico = 1;  
      strcpy(noti.noticia, mensajes1[i]);
      write(fd, &noti , sizeof(newp));
      noti.topico = 2;
      strcpy(noti.noticia, mensajes2[i]);
      write(fd, &noti , sizeof(newp));
      sleep(2);
    }
  
 
   exit(0);

}





