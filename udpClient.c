/*
void recibirMensajes(int conexion, struct sockaddr_in servaddr) {
    char mensaje[100];
    //while (1) {
        int recibir = recvfrom(conexion,mensaje,sizeof(mensaje),0,(struct sockaddr *)&servaddr, &sin_size);
        if(recibir >0){
            printf("%s\n", mensaje);
        }
    //}
}

void enviarMensajes(int conexion, struct sockaddr_in servaddr) {
    int n;
    char mensaje[100];
    //while(1){
        n=0;
        while ((mensaje[n++] = getchar()) != '\n');
        sendto(conexion,mensaje,sizeof(mensaje),0,(struct sockaddr *)&servaddr,sin_size);
        if ((strncmp(mensaje, "exit", 4)) == 0) { 
            printf("Client Exit...\n"); 
        }
    //}
}
*/

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include<netdb.h>

#define PORT	 5555


int main() { 
	iint sockfd;
    struct sockaddr_in servaddr, cliente;
    while(1){
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("UDP Client: ERROR while creating the socket.\n");
        exit(1);
    }
    bzero(&servaddr,sizeof(struct sockaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_addr.s_addr= INADDR_ANY; //inet_addr("127.0.0.1");
    servaddr.sin_port=htons(PORT);

    bzero(&cliente,sizeof(struct sockaddr));
    cliente.sin_family=AF_INET;
    cliente.sin_addr.s_addr=INADDR_ANY;
    printf("%s\n", argv[1]);
    cliente.sin_port=htons(atoi(argv[1]));
    if((bind(sockfd,(struct sockaddr *)&cliente,sizeof(cliente)))!=0){
        printf("client socket bind failed...\n");
        exit(0);
    }
    char nombre[16];
    strcpy(nombre, argv[2]);
    printf("Conectado con %s:%d\n",inet_ntoa(servaddr.sin_addr),htons(servaddr.sin_port));
    sendto(sockfd, nombre,sizeof(nombre),0,(struct sockaddr *)&servaddr,sin_size);
    pid_t childpid;
    for(;;){
        if((childpid = fork()) == 0){
            recibirMensajes(sockfd, servaddr);
        }
        if(enviarMensajes(sockfd, servaddr) == 0){
            exit(0);
        }else{
            break;
        }
    }
    close(sockfd);
    argv[1] = "4444";
    }
} 

