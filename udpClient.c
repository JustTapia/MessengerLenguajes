#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include<netdb.h>

#define PORT 5555


void recibirMensajes(int conexion, struct sockaddr_in servaddr, socklen_t sin_size) {
    char mensaje[512];
    while (1) {
        int recibir = recvfrom(conexion,mensaje,sizeof(mensaje),0,(struct sockaddr *)&servaddr, &sin_size);
        if(recibir >0){
            printf("%s\n", mensaje);
        }
    }
}

void comandos(int conexion, struct sockaddr_in servaddr, socklen_t sin_size){
    char mensaje[512];
    int j = 0, i = -1;

    while(1){

        *mensaje = '\0';
        i = -1;

        while(*(mensaje+i)!=10){
        i++;
        scanf("%c", mensaje+i);
        }

        *(mensaje+i)='\0';

        if( *mensaje != '-'){
            printf("Texto Invalido\n");

        }else if( *(mensaje + 1) == 's') { //enviar mensaje, comando -s
            if( *(mensaje + 2) == ' '){
                enviarMensaje(mensaje + 3, conexion, servaddr, sin_size);
            }else{
                enviarMensaje(mensaje + 2,  conexion, servaddr, sin_size);
            }

        }else if( *(mensaje + 1) == 'p'){ // cambiar puerto, comando -p
            if( *(mensaje + 2) == ' '){
                cambiarPuerto(mensaje + 3, conexion, servaddr, sin_size);
            }else{
                cambiarPuerto(mensaje + 2, conexion, servaddr, sin_size);
            }

        }else if( *(mensaje + 1) == 'q'){   // salir, comando -q
            printf("Muchas gracias por usar nuestro sistema!\n");
            break; 
        }else{
            printf("Comando Inexistente\n");
        }
    }
}

void enviarMensaje(char * mensaje, int conexion, struct sockaddr_in servaddr, socklen_t sin_size) {
    int n;
    //while(1){
        n=0;
        while ((mensaje[n++] = getchar()) != '\n');
        sendto(conexion, mensaje,sizeof(mensaje),0,(struct sockaddr *)&servaddr,sin_size);
        if ((strncmp(mensaje, "exit", 4)) == 0) { 
            printf("Client Exit...\n"); 
        }
    //}
}

void cambiarPuerto(char *mensaje, int conexion, struct sockaddr_in servaddr, socklen_t sin_size){
    char archtext[64];
    int i = 0, j = 0;
    FILE *archivo = fopen("arch.conf", "r");
    while(( *(archtext + i) = fgetc(archivo)) != 10){
        i++;
    }
    *(archtext + i) = '\n';
    i++;

    while(j<8){
        *(archtext + i) = fgetc(archivo);
        j++;
        i++;
    }

    j = 0;
    while(j < 4){
        *(archtext + i) = *(mensaje + j);
        i++;
        j++;
    }

    *(archtext+i) = '\0';
    fclose(archivo);

    archivo = fopen("arch.conf", "w");
    fprintf(archivo, "%s", archtext);
    fclose(archivo);
}

int main() { 
    char tmp, ip[15], sPuerto[2], nombre[16];
    int i = -1, j = 0;
    FILE *archivo = fopen("arch.conf", "r");
    while(j<4){
        tmp = fgetc(archivo);
        j++;
    }
    j = 0;
    while((tmp = fgetc(archivo)) != 10){
        *(ip+j) = tmp;
        j++;
    }
    *(ip+j) = '\0';
    j = 0;
    while(j<8){
        tmp = fgetc(archivo);
        j++;
    }
    j = 0;
    while((tmp = fgetc(archivo)) != EOF){
        *(sPuerto+j) = tmp;
        j++;
    }
    *(sPuerto+j) = '\0';
    fclose(archivo);
    printf("Por favor ingrese su nombre de usuario para su registro:\n");
    while(*(nombre+i)!=10){
        i++;
        scanf("%c", nombre+i);
    }
    *(nombre+i)='\0';


	int sockfd;
    struct sockaddr_in servaddr, cliente;
    //while(1){
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
    cliente.sin_port=htons(atoi(sPuerto));

    socklen_t sin_size = sizeof(struct sockaddr);
    sendto(sockfd, nombre,sizeof(nombre),0,(struct sockaddr *)&servaddr,sin_size);
    pid_t childpid =fork();
    if(childpid == 0){
        recibirMensajes(sockfd, servaddr, sin_size);
    }
    if(childpid >0){
        comandos(sockfd, servaddr, sin_size);
    }else{
        exit(0);
    }
    close(sockfd);
    //}
} 

