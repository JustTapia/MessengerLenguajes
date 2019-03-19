#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/wait.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<netdb.h>
#define PORT 23671
#define PORT2 4444

typedef struct clienteSocket {
    struct sockaddr_in *cli;
    struct clienteSocket* anterior;
    struct clienteSocket* siguiente;
    char nombre[16];
} clienteSocket;

clienteSocket *newSocket(char* nombre, struct sockaddr_in *client) {
    clienteSocket *ptr = malloc(sizeof(clienteSocket));
    ptr->cli = client;
    ptr->anterior = NULL;
    ptr->siguiente = NULL;
    strcpy(ptr->nombre, nombre);
    return ptr;
}

clienteSocket *servidorRaiz;
socklen_t sin_size = sizeof(struct sockaddr);

void  imprimirLista(clienteSocket  *SharedMem)
{
     printf(" Lista: \n");
     while(SharedMem != NULL){
        printf("%s   %i\n",SharedMem->nombre, htons(SharedMem->cli->sin_port));
        SharedMem = SharedMem->siguiente;
    }
     printf("  Fin de lista\n");
}

char * buscarNombre(struct sockaddr_in * cli){
    clienteSocket *temp = servidorRaiz;
    while (temp != NULL) {
        if (temp->cli == cli) {
            return temp->nombre;
        }
        temp = temp->siguiente;
    }
    return NULL;
}

int enviarMensaje(char *nombre, char *mensaje, int socket2, struct sockaddr_in * cli) {
    char from[] = "From ";
    char *nombreFrom;
    clienteSocket *temp = servidorRaiz->siguiente;
    while (temp != NULL) {
        if (strcmp(nombre, temp->nombre)==0) {
            nombreFrom = buscarNombre(cli);
            if(nombreFrom == NULL){
                printf("%s\n", "SI");
                return 1;
            }
            strcat(from, nombreFrom);
            strcat(from, ": ");
            strcat(from, mensaje);
            sendto(socket2,from,strlen(from),MSG_CONFIRM,(struct sockaddr *)(temp->cli),sin_size);
            return 0;
        }
        temp = temp->siguiente;
    }
    return 1;
}

void clienteDesconectado(clienteSocket *socket){
    if (socket->siguiente == NULL) {
        socket->anterior->siguiente = NULL;
    } else {
        socket->anterior->siguiente = socket->siguiente;
        socket->siguiente->anterior = socket->anterior;
    }
}

void agregarCliente(char *nombre, struct sockaddr_in * cli){
    clienteSocket *sckt = newSocket(nombre, cli);
    clienteSocket *temp = servidorRaiz;
    while (temp->siguiente != NULL) {
        temp = temp->siguiente;
    }
    sckt->anterior = temp;
    temp->siguiente = sckt;
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


void procesoCliente(int socket2, char *nombre){
    char buff[100];
    struct sockaddr_in cliente;
    for(;;){
        bzero(buff,sizeof(buff));
        recvfrom(socket2,buff,sizeof(buff), 0,(struct sockaddr *)&cliente, &sin_size);
        imprimirLista(servidorRaiz);
        int recibido = remitenteYmensaje(buff, socket2, &cliente);
        char mensaje[100];
        if(recibido!= 0){
            strcpy(mensaje, "\nNo se puede enviar el mensaje\n");
            sendto(socket2,mensaje,sizeof(mensaje),0,(struct sockaddr *)&cliente,sin_size);
        }else{
            strcpy(mensaje, "\nMensaje enviado\n");
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

    servidorRaiz = newSocket("servidor", &servaddr2);
    printf("\nUDP Server: done binding.");
    pid_t pid;
    int n;
    while (1) {
        bzero(buff,sizeof(buff));
        recvfrom(socket1,buff,sizeof(buff),0,(struct sockaddr *)&cliente, &sin_size);
        printf("\nConectando con %s:%d\n", inet_ntoa(cliente.sin_addr),htons(cliente.sin_port));
        printf("%s\n", buff);
        agregarCliente(buff, &cliente);
        printf("%d\n", cliente.sin_port);
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




