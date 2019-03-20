#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<netdb.h>
#define PORT 23671
#define PORT2 4444
#define GRN  "\x1B[32m"
#define RESET "\x1B[0m"

socklen_t sin_size = sizeof(struct sockaddr);

void recibirMensajes(int conexion, struct sockaddr_in *servaddr) {
    char mensaje[100];
    while (1) {
        int recibir = recvfrom(conexion,mensaje,sizeof(mensaje),0,(struct sockaddr *)servaddr, &sin_size);
        if(recibir > 0){
            printf("%s\n", mensaje);
        }
        else if (recibir == 0){
            break;
        }
    }
}

int enviarMensajes(int conexion, struct sockaddr_in *servaddr) {
    int n;
    //bool salir;
    char mensaje[100];
    while(1){
        n=0;
        while ((mensaje[n++] = getchar()) != '\n');
        *(mensaje +n -1) = '\0';
        sendto(conexion,mensaje,sizeof(mensaje),0,(struct sockaddr *) servaddr,sin_size);
        if ((strncmp(mensaje, "exit", 4)) == 0) { 
            printf("Client Exit...\n"); 
            return 0;
        }
    }
}


int main(int argc, char const *argv[])
{
    int sockfd;
    struct sockaddr_in servaddr, cliente, servaddr2;
    while(1){
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("UDP Client: ERROR while creating the socket.\n");
        exit(1);
        }
        bzero(&servaddr,sizeof(struct sockaddr));
        servaddr.sin_family=AF_INET;
        servaddr.sin_addr.s_addr= INADDR_ANY; //inet_addr("127.0.0.1");
        servaddr.sin_port=htons(PORT);

        bzero(&servaddr2,sizeof(struct sockaddr));
        servaddr2.sin_family=AF_INET;
        servaddr2.sin_addr.s_addr= INADDR_ANY; //inet_addr("127.0.0.1");
        servaddr2.sin_port=htons(PORT2);
        
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
                recibirMensajes(sockfd, &servaddr2);
            }
            if(enviarMensajes(sockfd, &servaddr2) == 0){
                exit(0);
            }else{
                break;
            }
        }
        close(sockfd);
        argv[1] = "4444";
    }
}
