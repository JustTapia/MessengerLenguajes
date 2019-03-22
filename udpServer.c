#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdint.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/wait.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<netdb.h>
#include <sys/mman.h>
#include <sys/types.h>
#define PORT 23671 //definimos puerto para socket 1
#define PORT2 4444 //definimos puerto para socket 2

typedef struct clienteSocket {
    int conectado;
    int puerto;
    struct in_addr ip;
    char *nombre;
} clienteSocket;

clienteSocket servidorRaiz;
static clienteSocket *clientes; //lista con todos los clientes
static int *nClientes; //cantidad de clientes
static char *nombres; //nombres de los clientes
socklen_t sin_size = sizeof(struct sockaddr); //tamaño de las estructuras sockaddr (utilizado en envío y recepción de mensajes)

//Guarda los datos de un cliente
clienteSocket newSocket(char *nombre, struct sockaddr_in *client) {
    strncpy(nombres + (*nClientes * 16), nombre, 16);
    clienteSocket ptr = {1, client->sin_port, client->sin_addr, nombres + (*nClientes * 16)};//malloc(sizeof(clienteSocket));
    return ptr;
}


//Imprime los clientes
void  imprimirLista()
{
     printf(" Lista: \n");
     for(int i = 0; i <= *nClientes; i++){
        printf("%s  %i\n",clientes[i].nombre, htons(clientes[i].puerto));
    }
     printf("  Fin de lista\n");
}

//Busca el nombre del remitente de un mensaje (mediante comparación de puertos)
char * buscarNombre(struct sockaddr_in  *cli){
    for(int i = 0; i <= *nClientes; i++){
        clienteSocket temp = clientes[i];
        if (temp.puerto == cli->sin_port) {
            return temp.nombre;
        }
    }
    return NULL;
}

//Enviamos un mensaje. Se busca al destinatario mediante comparación de nombre
int enviarMensaje(char *nombre, char *mensaje, int socket2, struct sockaddr_in * cli) {
    char from[100];
    char *nombreFrom;
    bzero(from, sizeof(from));
    struct sockaddr_in cliente2;
    for(int i = 0; i <= *nClientes; i++){
        clienteSocket temp = clientes[i];
        if (strcmp(nombre, temp.nombre) == 0) {
        	printf("%d\n", temp.conectado);
            if(temp.conectado == 0){
                return 1;
            }
            nombreFrom = buscarNombre(cli);
            if(nombreFrom == NULL){
                return 1;
            }
            //Forma el mensaje "From nombreUsuario: mensaje"
            strcat(from, "From ");
            strcat(from, nombreFrom);
            strcat(from, ": ");
            strcat(from, mensaje);
            strcat(from, "\0");
            //crea una estructura sockaddr con los datos guardados del destinatario para enviar el mensaje
            cliente2.sin_family=AF_INET;
            cliente2.sin_addr = temp.ip;
            cliente2.sin_port = temp.puerto;
            sendto(socket2,from,strlen(from),MSG_CONFIRM,(struct sockaddr *)&cliente2,sizeof(cliente2));
            return 0;
        }
    }
    return 1;
}

//separa el remitente del mensaje (formato-> nombreUsuarioDestinatario: mensaje) para, seguidamente, enviar el mensaje
int remitenteYmensaje(char *buffer, int socket2, struct sockaddr_in * cli){
    int i = 0;
    int j = 0;
    char nombre[16];
    char mensaje[100];
    while(*(buffer+i)!=':'){ //busca el nombre de usuario
        char c = *(buffer+i);
        *(nombre+i) = c;
        i++;
    }
    *(nombre+i)='\0';
    i++;
    while(*(buffer+i)!= '\n'){ //busca el mensaje
        char c = *(buffer+i);
        *(mensaje+j) = c;
        i++; j++;
    }
    *(mensaje+j) = '\0';
    return enviarMensaje(nombre, mensaje, socket2, cli);
}


//Cuando se desconecta un usuario, su información es incializada (en 0) en la lista de clientes
void desconectarUsuario(char *buffer, int socket2, struct sockaddr_in * cli){
    clienteSocket temp;
    struct sockaddr_in cliente2;
    for(int i = 0; i <= *nClientes; i++){
        temp = clientes[i];
        if (temp.puerto == cli->sin_port) {
            break;
        }
    }
    bzero(&temp.conectado , sizeof(int));
    cliente2.sin_family=AF_INET;
    cliente2.sin_addr = temp.ip;
    cliente2.sin_port = temp.puerto;
    sendto(socket2,"exit",4,MSG_CONFIRM,(struct sockaddr *)&cliente2,sizeof(cliente2));
    exit(1);
}

//proceso para manejar cada cliente
void procesoCliente(int socket2, char *nombre){
    char buff[100];
    struct sockaddr_in cliente, cliente2; //entra un cliente
    for(;;){
        int recibido = 0;
        bzero(buff,sizeof(buff));
        recvfrom(socket2,buff,sizeof(buff), 0,(struct sockaddr *)&cliente, &sin_size); //recibe mensaje de cliente
        //imprimirLista();
        if(strncmp(buff, "exit", 4) == 0){
            desconectarUsuario(buff, socket2, &cliente); //cliente se deconecta, palabra clave: "exit"
        }else{
            recibido = remitenteYmensaje(buff, socket2, &cliente); //envía un mensaje
        }
        char mensaje[100];
        if(recibido!= 0){
            strcpy(mensaje, "\nNo se puede enviar el mensaje\n\0"); //error: cliente desconectado o no existe
            sendto(socket2,mensaje,sizeof(mensaje),0,(struct sockaddr *)&cliente,sin_size);
        }
    }
}

void main(int argc, char const *argv[]){

    struct sockaddr_in servaddr, cliente, servaddr2;
    int bytes_read;
    char buff[100];
    int socket1, socket2;
    bzero(&servaddr, sizeof(servaddr));
    bzero(&cliente, sizeof(cliente));
    bzero(&servaddr2, sizeof(servaddr2));
    
    //Socket 1
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
    
    //Socket 2
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

    //Shared Memory
    clientes = mmap(NULL, 100*sizeof(clienteSocket), PROT_READ | PROT_WRITE, 
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0); 
    nClientes = mmap(NULL, sizeof(*nClientes), PROT_READ | PROT_WRITE, 
                   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    nombres = mmap(NULL, sizeof(char)*1600, PROT_READ | PROT_WRITE, 
                   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    bzero(clientes,100*sizeof(clienteSocket));
    bzero(clientes,sizeof(*nClientes));
    bzero(clientes,1600*sizeof(char));
    *nClientes = 0;
    servidorRaiz = newSocket("servidor", &servaddr2);
    clientes[*nClientes]= servidorRaiz; //primer elemento de la lista es el servidor (equivale a inicializar la lista)
    printf("\nUDP Server: done binding.");
    pid_t pid;
    int n;
    while (1) {
        bzero(buff,sizeof(buff));
        recvfrom(socket1,buff,sizeof(buff),0,(struct sockaddr *)&cliente, &sin_size); //recibe el nombre de usuario del cliente
        printf("\nConectando con %s:%d\n", inet_ntoa(cliente.sin_addr),htons(cliente.sin_port)); //IP y puerto del cliente
        printf("%s\n", buff);
        (*nClientes)++; //sumo cantidad de clientes
        clienteSocket c = newSocket(buff, &cliente);
        clientes[*nClientes]= c; //agregamos estructura con los datos del cliente a la lista
        pid = fork();
        if(pid<0){
            printf("%s\n", "ERROR forking");
        }
        if(pid == 0){
            //proceso hijo
            procesoCliente(socket2, buff);
        }
    }     
    close(socket1);
    close(socket2);
    exit(0);
}
