#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/wait.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<netdb.h>
#include <sys/mman.h>
#include <sys/types.h>
#define PORT 23671
#define PORT2 4444

typedef struct clienteSocket {
    int desconectado;
    unsigned short int puerto;
    struct in_addr ip;
    char *nombre;
} clienteSocket;

clienteSocket servidorRaiz;
static clienteSocket *clientes;
static int *nClientes; 
static char *nombres;
socklen_t sin_size = sizeof(struct sockaddr);

clienteSocket newSocket(char *nombre, struct sockaddr_in *client) {
    strncpy(nombres + (*nClientes * 16), nombre, 16);
    clienteSocket ptr = {0, client->sin_port, client->sin_addr, nombres + (*nClientes * 16)};//malloc(sizeof(clienteSocket));
    printf("%s\n", ptr.nombre);
    return ptr;
}



void  imprimirLista()
{
     printf(" Lista: \n");
     for(int i = 0; i <= *nClientes; i++){
        printf("%s  %i\n",clientes[i].nombre, htons(clientes[i].puerto));
    }
     printf("  Fin de lista\n");
}

char * buscarNombre(struct sockaddr_in  *cli){
    for(int i = 0; i <= *nClientes; i++){
        clienteSocket temp = clientes[i];
        if (temp.puerto == cli->sin_port) {
            return temp.nombre;
        }
    }
    return NULL;
}

int enviarMensaje(char *nombre, char *mensaje, int socket2, struct sockaddr_in * cli) {
    char from[100];
    char *nombreFrom;
    struct sockaddr_in cliente2;
    for(int i = 0; i <= *nClientes; i++){
        clienteSocket temp = clientes[i];
        if (strcmp(nombre, temp.nombre)==0) {
            if(temp.desconectado == 1){
                return 1;
            }
            nombreFrom = buscarNombre(cli);
            if(nombreFrom == NULL){
                return 1;
            }
            bzero(from,sizeof(from));
            strcat(from, "From ");
            strcat(from, nombreFrom);
            strcat(from, ": ");
            strcat(from, mensaje);
            strcat(from, "\0");
            cliente2.sin_family=AF_INET;
            cliente2.sin_addr = temp.ip;
            cliente2.sin_port = temp.puerto;
            sendto(socket2,from,strlen(from),MSG_CONFIRM,(struct sockaddr *)&cliente2,sizeof(cliente2));
            return 0;
        }
    }
    return 1;
}


int remitenteYmensaje(char *buffer, int socket2, struct sockaddr_in * cli){
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
    return enviarMensaje(nombre, mensaje, socket2, cli);
}

void cambiarPuerto(char *buffer, struct sockaddr_in * cli){
    clienteSocket temp;
    for(int i = 0; i <= *nClientes; i++){
        temp = clientes[i];
        if (temp.puerto == cli->sin_port) {
            break;
        }
    }
    temp.puerto = atoi(buffer);
}

void desconectarUsuario(char *buffer, struct sockaddr_in * cli){
    clienteSocket temp;
    for(int i = 0; i <= *nClientes; i++){
        temp = clientes[i];
        if (temp.puerto == cli->sin_port) {
            break;
        }
    }
    temp.desconectado = 1;
    printf("%s\n%d\n", temp.nombre, temp.puerto);
}

void procesoCliente(int socket2, char *nombre){
    char buff[100];
    struct sockaddr_in cliente, cliente2;
    for(;;){
        int recibido = 0;
        bzero(buff,sizeof(buff));
        recvfrom(socket2,buff,sizeof(buff), 0,(struct sockaddr *)&cliente, &sin_size);
        //imprimirLista();
        if(strncmp(buff, "Puerto", 6) == 0){
            cambiarPuerto(buff, &cliente);
        }else if(strncmp(buff, "exit", 4) == 0){
            desconectarUsuario(buff, &cliente);
        }else{
            recibido = remitenteYmensaje(buff, socket2, &cliente);
        }
        char mensaje[100];
        if(recibido!= 0){
            strcpy(mensaje, "\nNo se puede enviar el mensaje\n\0");
            sendto(socket2,mensaje,sizeof(mensaje),0,(struct sockaddr *)&cliente,sin_size);
        }
    }
}

void main(int argc, char const *argv[])
{
    struct sockaddr_in servaddr, cliente, servaddr2;
    int bytes_read;
    char buff[100];
    int socket1, socket2;
    
    if ((socket1 = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("UDP Server: ERROR creating the socket.\n");
        exit(1);
    }
    printf("Socket successfully created..\n");
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_addr.s_addr=INADDR_ANY;
    servaddr.sin_port=htons(PORT);

    printf("\nUDP Server: server socket binding...");
    if (bind(socket1, (struct sockaddr *) &servaddr, sizeof(struct sockaddr)) == -1) {
        perror("UDP Server: ERROR binding the socket.\n");
        exit(1);
    }

    if ((socket2 = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("UDP Server2: ERROR creating the socket.\n");
        exit(1);
    }
    printf("Socket successfully created..\n");
    bzero(&servaddr2,sizeof(servaddr2));
    servaddr2.sin_family=AF_INET;
    servaddr2.sin_addr.s_addr=INADDR_ANY;
    servaddr2.sin_port=htons(PORT2);

    printf("\nUDP Server2: server socket2 binding...");
    if (bind(socket2, (struct sockaddr *) &servaddr2, sizeof(struct sockaddr)) == -1) {
        perror("UDP Server2: ERROR binding the socket2.\n");
        exit(1);
    }
    printf("Socket2 successfully created..\n");

    clientes = mmap(NULL, 100*sizeof(clienteSocket), PROT_READ | PROT_WRITE, 
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    nClientes = mmap(NULL, sizeof(*nClientes), PROT_READ | PROT_WRITE, 
                   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    nombres = mmap(NULL, sizeof(char)*1600, PROT_READ | PROT_WRITE, 
                   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *nClientes = 0;
    servidorRaiz = newSocket("servidor", &servaddr2);
    clientes[*nClientes]= servidorRaiz;
    printf("\nUDP Server: done binding.");
    pid_t pid;
    int n;
    while (1) {
        bzero(buff,sizeof(buff));
        recvfrom(socket1,buff,sizeof(buff),0,(struct sockaddr *)&cliente, &sin_size);
        printf("\nConectando con %s:%d\n", inet_ntoa(cliente.sin_addr),htons(cliente.sin_port));
        printf("%s\n", buff);
        (*nClientes)++;
        clienteSocket c = newSocket(buff, &cliente);
        clientes[*nClientes]= c;
        pid = fork();
        if(pid<0){
            printf("%s\n", "ERROR forking");
        }
        if(pid == 0){
            procesoCliente(socket2, buff);
        }
    }     
    close(socket1);
    close(socket2);
    exit(0);
}