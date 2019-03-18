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
	int socket_fd, n;
	struct sockaddr_in server_addr;
	struct hostent *host;
	host = gethostbyname("127.0.0.1");

	if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("UDP Client: ERROR while creating the socket.\n");
		exit(1);
	}
	bzero(&server_addr,sizeof(struct sockaddr));
    server_addr.sin_family=AF_INET;
    //servaddr.sin_addr.s_addr=inet_addr("127.0.0.1");
    server_addr.sin_port=htons(PORT);
    bcopy((char *)host->h_addr, (char *)&server_addr.sin_addr.s_addr, sizeof(host->h_length));

	int count;
	char buffer[128];
	pid_t childpid;
	while (1) {
		socklen_t sin_size = sizeof(struct sockaddr);
		if((childpid = fork()) == 0){	
        	n = recvfrom(socket_fd, buffer, sizeof(buffer), MSG_WAITALL, (struct sockaddr *) &server_addr, &sin_size);
        	if (n < 0) {
        		perror("UDP Client: ERROR while reading response from server socket.\n");
			}else{
				buffer[n] = '\0';
				printf("UDP Client: received response -> %s\n", buffer);
        	} 
        }
        char mensaje[100];
        n=0;
        while ((mensaje[n++] = getchar()) != '\n');
        sendto(socket_fd, mensaje, strlen(mensaje), MSG_CONFIRM, (struct sockaddr *) &server_addr, sin_size);
	}
	close(socket_fd); 
	return 0;
} 

