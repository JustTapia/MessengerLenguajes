#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#define PORT 4444
#define GRN  "\x1B[32m"
#define RESET "\x1B[0m"

void recibirMensajes(int conexion) {
  char mensaje[100];
  while (1) {
    int recibir = read(conexion, mensaje, sizeof(mensaje)); 
    if(recibir >0){
      printf(GRN "%s\n" RESET, mensaje);
    }
    else if (recibir == 0){
      break;
    }
  }
}

void enviarMensajes(int conexion) {
  int n;
  char mensaje[100];
  while(1){
    n=0;
    while ((mensaje[n++] = getchar()) != '\n');
    write(conexion, mensaje, sizeof(mensaje));
    if ((strncmp(mensaje, "exit", 4)) == 0) { 
      printf("Client Exit...\n"); 
      exit(0);
    }
  }
}


int main(int argc, char **argv){
  if(argc<1)
  {
    printf("nombre\n");
    return 1;
  }
  struct sockaddr_in server, cliente; //Estructura con información para la conexión
  struct hostent *servidor; //Estructura con información del host
  servidor = gethostbyname("localhost");//"192.168.0.30 (ip de servidor)
  if(servidor == NULL)
  {
    printf("Host erróneo\n");
    return 1;
  }
  int conexion;
  char nombre[16];
  strcpy(nombre, argv[1]);//nombre de usuario
  char buffer[100]; //mensaje en pantalla
  conexion = socket(AF_INET, SOCK_STREAM, 0);
  bzero((char *)&server, sizeof((char *)&server));
  server.sin_family = AF_INET;
  server.sin_port = htons(PORT); //Puerto del servidor
  bcopy((char *)servidor->h_addr, (char *)&server.sin_addr.s_addr, sizeof(servidor->h_length));
  cliente.sin_family = AF_INET;
  cliente.sin_addr.s_addr = INADDR_ANY;
  //cliente.sin_port = htons(12010); //asignar directamente un puerto para el cliente
  if(bind(conexion, (struct sockaddr *)&cliente, sizeof(cliente)) < 0)
  {
    printf("Error al asociar el puerto a la conexion\n");
    close(conexion);
    return 1;
  }
  //cliente.sin_addr = *((struct in_addr *)servidor->h_addr);
  //inet_aton(argv[1],&cliente.sin_addr); //<--alguna de estas dos funciones
  if(connect(conexion,(struct sockaddr *)&server, sizeof(server)) < 0)
  {
    printf("Error conectando con el host\n");
    close(conexion);
    return 1;
  }
  printf("Conectado con %s:%d\n",inet_ntoa(server.sin_addr),htons(server.sin_port));
  write(conexion, &nombre, sizeof(nombre));
  pid_t childpid;
  for(;;){
    if((childpid = fork()) == 0){
      recibirMensajes(conexion);
    }
    enviarMensajes(conexion);
}
return 0;
}



