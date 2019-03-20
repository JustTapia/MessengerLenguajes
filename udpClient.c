#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<netdb.h>
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
        bzero(mensaje, sizeof(mensaje));
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
        if( strncmp(mensaje, "Puerto", n) == 0){ // cambiar puerto, comando -p
            cambiarPuerto(mensaje, conexion, servaddr);
        }else{
            sendto(conexion,mensaje,sizeof(mensaje),0,(struct sockaddr *) servaddr,sin_size);
            if ((strncmp(mensaje, "exit", 4)) == 0) { 
                printf("Muchas gracias por usar nuestro sistema!\n"); 
                return 0;
            }
        }
    }
}
void cambiarPuerto(char *mensaje, int conexion, struct sockaddr_in *servaddr){
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
    sendto(conexion,mensaje,sizeof(mensaje),0,(struct sockaddr *) servaddr,sin_size);
    printf("Puerto cambiado!\n");
}

int main(){
    char tmp, ip[15], sPuerto[5], nombre[16], PORT[6], PORT2[6];
    int i = -1, j = 0;
    FILE *archivo = fopen("arch.conf", "r");
    while(j<4){
        tmp = fgetc(archivo);
        j++;
    }
    j = 0;
    while((tmp = fgetc(archivo)) != ':'){
        *(ip+j) = tmp;
        j++;
    }
    *(ip+j) = '\0';

    j = 0;
    while((tmp = fgetc(archivo)) != ':'){
        *(PORT+j) = tmp;
        j++;
    }
    *(PORT+j) = '\0';
    j = 0;
    while((tmp = fgetc(archivo)) != 10){
        *(PORT2+j) = tmp;
        j++;
    }
    *(PORT2+j) = '\0';
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
    struct sockaddr_in servaddr, cliente, servaddr2;
    while(1){
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("UDP Client: ERROR while creating the socket.\n");
        exit(1);
        }
        bzero(&servaddr,sizeof(struct sockaddr));
        servaddr.sin_family=AF_INET;
        servaddr.sin_addr.s_addr= INADDR_ANY; //inet_addr("127.0.0.1");
        servaddr.sin_port=htons(atoi(PORT));

        bzero(&servaddr2,sizeof(struct sockaddr));
        servaddr2.sin_family=AF_INET;
        servaddr2.sin_addr.s_addr= INADDR_ANY; //inet_addr("127.0.0.1");
        servaddr2.sin_port=htons(atoi(PORT2));
        
        bzero(&cliente,sizeof(struct sockaddr));
        cliente.sin_family=AF_INET;
        cliente.sin_addr.s_addr=INADDR_ANY;
        cliente.sin_port=htons(atoi(sPuerto));
        if((bind(sockfd,(struct sockaddr *)&cliente,sizeof(cliente)))!=0){
            printf("client socket bind failed...\n");
            exit(0);
        }
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
        exit(0);
    }
    //close(sockfd);
}