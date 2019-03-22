#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<netdb.h>

socklen_t sin_size = sizeof(struct sockaddr);

/*
cambia el puerto en el archivo de configuración
*/
void cambiarPuerto(char *mensaje, int conexion, struct sockaddr_in *servaddr){
    char archtext[64];
    int i = 0, j = 0;
    FILE *archivo = fopen("arch.conf", "r");
    while(( *(archtext + i) = fgetc(archivo)) != 10){
        i++;
    }
    *(archtext + i) = '\n';
    i++;

    while(j<8){ //busca la cadena "Puerto: "
        *(archtext + i) = fgetc(archivo);
        j++;
        i++;
    }

    j = 0;
    while(j < 5){ //coloca un puerto de 5 caracteres
        *(archtext + i) = *(mensaje + j);
        i++;
        j++;
    }

    *(archtext+i) = '\0';
    fclose(archivo);
    archivo = fopen("arch.conf", "w");
    fprintf(archivo, "%s", archtext);
    fclose(archivo);
    printf("Puerto cambiado!\n");
}

/*
recibe los mensajes enviados por el servidor
se utiliza el segundo socket del servidor
'conexion' es el socket del cliente
*/
void recibirMensajes(int conexion, struct sockaddr_in *servaddr) {
    char mensaje[100];
    while (1) {
        int recibir = recvfrom(conexion,mensaje,sizeof(mensaje),0,(struct sockaddr *)servaddr, &sin_size); //recibe mensaje del servidor
        if(recibir > 0){
            if(strncmp(mensaje, "exit", 4) ==0){
                exit(0);
            }
            printf("\033[0;32m"); //mensaje en color verde
            printf("%s", mensaje);
            printf("\e[0m"); //reseteamos el color
            printf("\n");
        }
        else if (recibir == 0){
            break;
        }
        bzero(mensaje, sizeof(mensaje)); //vaciamos el mensaje
    }
}

/*
envía los mensajes al servidor
se utiliza el segundo socket del servidor
'conexion' es el socket del cliente
*/
int enviarMensajes(int conexion, struct sockaddr_in *servaddr) {
    int n;
    char mensaje[100];
    while(1){
        n=0;
        while ((mensaje[n++] = getchar()) != '\n'); //mensaje escrito en consola
        *(mensaje +n -1) = '\0';
        sendto(conexion,mensaje,sizeof(mensaje),0,(struct sockaddr *) servaddr,sin_size); //envía mensaje al servidor
        if ((strncmp(mensaje, "exit", 4)) == 0) { 
        	printf("Muchas gracias por usar nuestro sistema!\n"); //cliente se desconecta
        	close(conexion); //cierro el socket
        	exit(0);
        }
    }
}

int main(){
    char tmp, ip[15], sPuerto[5], nombre[16], PORT[6], PORT2[6], sn[3];
    int i = -1, j = 0;
    FILE *archivo = fopen("arch.conf", "r");
    while(j<4){ //busca la cadena "IP: "
        tmp = fgetc(archivo);
        j++;
    }
    j = 0;
    while((tmp = fgetc(archivo)) != ':'){ //busca el ip del servidor
        *(ip+j) = tmp;
        j++;
    }
    *(ip+j) = '\0';

    j = 0;
    while((tmp = fgetc(archivo)) != ':'){ //busca el primer puerto del servidor
        *(PORT+j) = tmp;
        j++;
    }
    *(PORT+j) = '\0';
    j = 0;
    while((tmp = fgetc(archivo)) != 10){ //busca el segundo puerto del servidor
        *(PORT2+j) = tmp;
        j++;
    }
    *(PORT2+j) = '\0';
    j = 0;
    while(j<8){ //busca la cadena "Puerto: "
        tmp = fgetc(archivo);
        j++;
    }
    j = 0;
    while((tmp = fgetc(archivo)) != EOF){ //busca el puerto del cliente
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

    /*
    antes de inciar, se da al cliente la opción de cambiar el puerto
    */
    printf("Desea cambiar su número de puerto? s/n\n");

    i = -1;
    while( *(sn + i) != '\n'){
        i++;
        scanf("%c", sn + i);
    }
    *(sn + i) = '\0';
    if (*sn == 's'){
        i = -1;
        printf("\nPor favor escriba el puerto que desea\n");
        while(*(sPuerto+i)!=10){
            i++;
            scanf("%c", sPuerto+i);
        }
        *(sPuerto+i)='\0';
    } 

    int sockfd; //socket del cliente
    struct sockaddr_in servaddr, cliente, servaddr2; //estructura para el cliente y los dos servidores

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("UDP Client: ERROR while creating the socket.\n");
        exit(1);
    }
    
    //Servidor socket 1
    bzero(&servaddr,sizeof(struct sockaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_addr.s_addr= inet_addr(ip); //asignamos ip del servidor leído en el archivo
    servaddr.sin_port=htons(atoi(PORT)); //asignamos primer puerto del servidor

    //Servidor socket 2
    bzero(&servaddr2,sizeof(struct sockaddr));
    servaddr2.sin_family=AF_INET;
    servaddr2.sin_addr.s_addr= inet_addr(ip);
    servaddr2.sin_port=htons(atoi(PORT2)); //asignamos segundo puerto del servidor
    
    //Cliente
    bzero(&cliente,sizeof(struct sockaddr));
    cliente.sin_family=AF_INET;
    cliente.sin_addr.s_addr=INADDR_ANY; //ip del cliente
    cliente.sin_port=htons(atoi(sPuerto)); //puerto del cliente
    if((bind(sockfd,(struct sockaddr *)&cliente,sizeof(cliente)))!=0){ //relacionamos el socket cliente con la estructura cliente
        printf("client socket bind failed...\n");
        exit(0);
    }
    printf("Conectado con %s:%d\n",inet_ntoa(servaddr.sin_addr),htons(servaddr.sin_port)); //Entra al servidor (socket 1)
    sendto(sockfd, nombre,sizeof(nombre),0,(struct sockaddr *)&servaddr,sin_size); //envía el nombre de usuario al servidor (socket 1)
    pid_t childpid;
    for(;;){
        if((childpid = fork()) == 0){
            //proceso hijo
            recibirMensajes(sockfd, &servaddr2);
        }
        //proceso padre
        if(enviarMensajes(sockfd, &servaddr2) == 0){
            exit(0);
        }else{
            break;
        }
    }
    close(sockfd);
    exit(0);
}
