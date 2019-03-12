#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<netdb.h>
#define PORT 4444

typedef struct clienteSocket {
	int conexion;
	struct clienteSocket* anterior;
    struct clienteSocket* siguiente;
    char nombre[16];
} clienteSocket;

clienteSocket *newSocket(int socket, char* nombre) {
	clienteSocket *ptr = malloc(sizeof(clienteSocket));
    ptr->conexion = socket;
    ptr->anterior = NULL;
    ptr->siguiente = NULL;
    strcpy(ptr->nombre, nombre);
    return ptr;
}

clienteSocket *servidorRaiz, *actual;

int enviarMensaje(char *nombre, char *mensaje) {
	clienteSocket *temp = servidorRaiz->siguiente;
	while (temp != NULL) {
		if (strcmp(nombre, temp->nombre)==0) {
			write(temp->conexion, mensaje, sizeof(mensaje));
            return 0;
        }
        temp = temp->siguiente;
    }
    return 1;
}

void clienteDesconectado(clienteSocket *socket){
	close(socket->conexion);
	if (socket->siguiente == NULL) {
		actual = socket->anterior;
		actual->siguiente = NULL;
	} else {
		socket->anterior->siguiente = socket->siguiente;
        socket->siguiente->anterior = socket->anterior;
    }
    free(socket);
}


int remitenteYmensaje(char *buffer){
	//printf("%s\n", buffer);
	int i = 0;
	int j = 0;
	char nombre[16];
	char mensaje[100];
	while(*(buffer+i)!=':'){
		char c = *(buffer+i);
		*(nombre+i) = c;
		i++;
	}
	*(nombre+i)='\0';
	i++;
	while(*(buffer+i)!= '\n'){
		char c = *(buffer+i);
		*(mensaje+j) = c;
		i++; j++;
	}
	*(mensaje+j) = '\0';
	//printf("%s\t", nombre);
	//printf("%s\n",mensaje);
	return enviarMensaje(nombre, mensaje);
}


void  ClientProcess(clienteSocket  *SharedMem)
{
     printf("   Client process started\n");
     while(SharedMem != NULL){
        printf("%s %i\n",SharedMem->nombre, SharedMem->conexion);
        SharedMem = SharedMem->siguiente;
    }
     printf("   Client is about to exit\n");
}

int main(){
  int conexionServidor, conexionCliente, puerto;
  socklen_t longc;
  struct sockaddr_in servidor, cliente;
  char buffer[100];
  pid_t childpid;
  conexionServidor = socket(AF_INET, SOCK_STREAM, 0);
  bzero((char *)&servidor, sizeof(servidor));
  servidor.sin_family = AF_INET;
  servidor.sin_port = htons(PORT);
  servidor.sin_addr.s_addr = INADDR_ANY;
  if(bind(conexionServidor, (struct sockaddr *)&servidor, sizeof(servidor)) < 0)
  {
    printf("Error al asociar el puerto a la conexion\n");
    close(conexionServidor);
    return 1;
  }
  listen(conexionServidor, 3);
  printf("A la escucha en el puerto %d\n", ntohs(servidor.sin_port));
  longc = sizeof(cliente);
  servidorRaiz = newSocket(conexionServidor, "servidor");
  actual = servidorRaiz;
  while(1){
  conexionCliente = accept(conexionServidor, (struct sockaddr *)&cliente, &longc);
  if(conexionCliente<0)
  {
    close(conexionServidor);
    return 0;
  }
  printf("Conectando con %s:%d\n", inet_ntoa(cliente.sin_addr),htons(cliente.sin_port));
  char nombre[16];
  read(conexionCliente, nombre, sizeof(nombre));
  clienteSocket *sckt = newSocket(conexionCliente, nombre);
  sckt->anterior = actual;
  actual->siguiente = sckt;
  actual = sckt;
  if((childpid = fork()) == 0){
      close(conexionServidor);
      int n;
      for(;;){
        //printf("%s\n", actual->nombre);
        //printf("%i\n", actual->conexion); 
        ClientProcess(servidorRaiz);
        bzero(buffer, sizeof(buffer));  
        read(conexionCliente, buffer, sizeof(buffer));
        if ((strncmp("exit", buffer, 4)) == 0) { 
          printf("Cliente desconectado\n"); 
          break; 
        }
        //write(conexionCliente, buffer, sizeof(buffer));
        int recibido = remitenteYmensaje(buffer);
        char mensaje[100];
        if(recibido == 1){
          strcpy(mensaje, "\nNo se puede enviar el mensaje\n");
          write(conexionCliente, mensaje, sizeof(mensaje));
        }else{
          strcpy(mensaje, "\nMensaje enviado\n");
          write(conexionCliente, mensaje, sizeof(mensaje));
        }
        /*n = 0; 
        while ((buffer[n++] = getchar()) != '\n'); 
        write(conexionCliente, buffer, sizeof(buffer));*/
      }
      ClientProcess(servidorRaiz);
      clienteDesconectado(sckt);
    }


  }
  return 0;
}

