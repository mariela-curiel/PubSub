// Autor: Mariela Curiel
// Descripcion: ejemplo de un Publicador/Suscriptor. En este archivo se encuentran dos hilos aparte del hilo principal.
// Un hilo lee noticias del publicador y la pone en un buffer,  el principal está pendiente de la conexión de nuevos suscriptores y otro
// hilo se encarga de recoger las noticias del buffer y enviarlas a los suscriptores. 
// El programa se invoca de la siguiente forma:
// $central pipesus pipepub
// El primer argumento es el pipe de comunicación con los suscriptores y el segundo con el publicador.
//  Solo se manejan dos tópicos y se guarda únicamente la última noticia de cada tópico. Solo fue probado con un publicador que envía 10
//  noticias asociadas a dos topicos. 
//  El suscriptor no necesariamente reciben todas las noticias, dependerá de su tiempo de creación. 

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "nom.h" 

// El tamaño del BUFFER donde se ponen las noticias

#define TAMBUF 10

// Prototipo de las funciones

void *take(newp *);
void *put(newp *);
void Publicador(int *fd);
void CerrarPipesS();
void BuscarUltima(int, int);
void ColocarSuscriptor(int, char*);

//Variables globales 

newp BUFFER[TAMBUF]; // buffer donde se pondrán las noticias para consumo inmediato.
newp LASTNEW[NTOP]; // buffer donde se guarda la última noticia de cada tópico. 
int globalfd[NSUSCR]; // se guardan los file descriptor de los pipes asociados a los suscriptores.
topicos suscr[NSUSCR]; // topicos y pid de cada suscriptor.
int finProd = FALSE;
int finCons = FALSE; // Para el fin de la lectura de las noticias
int pcons=0, pprod=0;
sem_t s, espacios, elementos; // semaforos para la implementacion del buffer




// Función que cierra los pipes por donde el central envía datos a los suscriptores. 
void CerrarPipesS() {

  int i;
  for(i = 0; i < NSUSCR; i++)
    if (globalfd[i] > 0) close(globalfd[i]);

}  


//Thread que lee del pipe del proceso publicador y escribe en el buffer de noticias. 
void Publicador(int *fd) {

  int cuantos; 
  newp new;
 
 do {
    cuantos = read (*fd, &new, sizeof(newp)); // recordar validar llamadas al sistema
    if (cuantos == 0) break;
    put(&new);
  } while (cuantos > 0);
 
  new.topico = -1;
  put(&new);
  printf("\n Se terminan de leer las noticias del Publicador");
  finProd = TRUE;
  pthread_exit(NULL);
}


// Se busca la ultima noticia de un determinado tópico y se envía al fd cuya posición de recibe como primer argumento.
void BuscarUltima(int pos, int topico) {

  if (LASTNEW[topico - 1].topico != -1) {
     printf("Buscando Ultima envia %s  \n", LASTNEW[topico -1].noticia);
     write(globalfd[pos], LASTNEW[topico -1].noticia, strlen(LASTNEW[topico-1].noticia) + 1);
  }  

}  


// Esta función la invoca el hilo consumidor (el que toma las noticias del buffer, para enviarlas a los suscriptores
void ColocarSuscriptor(int topico, char *noticia) {

  int i=0, salir=FALSE;

  // Coloca la ultima noticia del topico en la estructura de datos correspondiente
   LASTNEW[topico-1].topico = topico;
   strcpy(LASTNEW[topico-1].noticia, noticia);

   
    // Enviar la noticia por el pipe correspondiente.
    while ((globalfd[i] != -1) && (salir == FALSE)) {       
      if (suscr[i].topico == topico) { // enviar la noticia por el pipe correspondiente
         write(globalfd[i], noticia, strlen(noticia) + 1);

      }
      i++;

     }

}  


// Funcion para tomar datos del buffer de noticias
void *take(newp *e) {

  newp temp,*pe;

  pe = e; 
  int i=0;

  for (;;) {
    
    sem_wait(&elementos);
    sem_wait(&s);
    memcpy(&temp, &pe[pcons], sizeof(newp));
    pe[pcons].topico = 0; // para indicar que la posición está vacia
    pcons= (pcons + 1) % TAMBUF;
    if (temp.topico  == -1) { // el ultimo elemento
      sem_post(&s);
      sem_post(&espacios);
      finCons == TRUE;
      break;
    } else {
      ColocarSuscriptor(temp.topico, temp.noticia);
      sem_post(&s);
      sem_post(&espacios);
    }  
    
  }
 
  printf("\n Thread consumidor termina \n");
  if (finProd == TRUE) CerrarPipesS();
  pthread_exit(NULL);
  

}  

// Funcion para colocar elementos del buffer. 
void *put(newp *e) {
      sem_wait(&espacios);
      sem_wait(&s);
      if (BUFFER[pprod].topico == 0) 
	memcpy(&BUFFER[pprod], e, sizeof(newp));
      pprod = (pprod + 1) % TAMBUF;
      sem_post(&s);
      sem_post(&elementos);
}  


int main (int argc, char **argv)
{
  int fd, fd1, pid, n, bytes, cuantos, creado,i, abiertos;
  datap datos;
  pthread_t thread[3];
 
   
  mode_t fifo_mode = S_IRUSR | S_IWUSR;

  // inicializacion de los semáforos siguiendo el algoritmo productor, consumidor
  sem_init(&s, 0, 1);
  sem_init(&espacios, 0, TAMBUF);
  sem_init(&elementos, 0, 0);
  
  // inicializacion de las estructuras de datos globales compartidas
  
  for (i=0; i < TAMBUF; i++) BUFFER[i].topico = 0;
  for (i=0; i < NSUSCR; i++) globalfd[i] = -1;
  for (i=0; i < NSUSCR; i++) suscr[i].topico = -1;
  for (i=0; i < NTOP; i++) LASTNEW[i].topico = -1;
  
 
  // Creacion del pipe del lado del publicador
    if (mkfifo (argv[2], fifo_mode) == -1) {
        perror("mkfifo");
        exit(1);
    }  

     fd1 = open (argv[2], O_RDONLY);
     if (fd1 == -1) {
         perror("pipe publicador");
         exit (0);
      }

  // Creacion del pipe del lado del suscriptor

     if (mkfifo (argv[1], fifo_mode) == -1) {
        perror("mkfifo");
        exit(1);
     }

     fd = open (argv[1], O_RDONLY);
     if (fd == -1) {
       perror("pipe suscriptor");
       exit (0);
     }


     // El primer hilo, lee las noticias del pipe del publicador y las coloca en el BUFFER de noticias, el segundo hilo toma las noticias
     // del BUFFER y las envía a los suscriptores. 
     
     pthread_create(&thread[0], NULL, (void*) Publicador, (void*)&fd1);        
     pthread_create(&thread[1], NULL, (void*) take, (void*)BUFFER);
      
     // El hilo principal se queda leyendo del pipe de los suscriptores.     
  
    
   
      for(i=0;;i++) {

       cuantos = read (fd, &datos, sizeof(datos));
   
       if (cuantos <= 0) break;
     
       do { 
          if ((globalfd[i] = open(datos.segundopipe, O_WRONLY)) == -1) {
             perror("Server Abriendo el segundo pipe ");
             printf("Se volvera a intentar despues\n");
             sleep(2);         
          } else creado = 1; 
       }  while (creado == 0);
       suscr[i].pid = datos.pid;
       suscr[i].topico = datos.topico;    
       BuscarUltima(i,suscr[i].topico);
    }   // Fin del ciclo infinito
    

  // Cerrar y eliminar el pipe
      close(fd);
      close(fd1);
      unlink(argv[1]);
      unlink(argv[2]);
}
