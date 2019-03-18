// Server side implementation of UDP client-server model 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h>
#include <sys/wait.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include<netdb.h>

#define PORT	5555


void procesoCliente(int sockfd, char* nombre){
    char buffer[512];
    int n;
    struct sockaddr_in cliaddr;
    int len = sizeof(cliaddr);
    while(1){
    	bzero(buffer, sizeof(buffer));
    	recvfrom(sockfd, buffer, sizeof(buffer), MSG_WAITALL, 
    		( struct sockaddr *) &cliaddr, &len); 
    	n= 0;
    	for(; buffer[n] != '\n'; n++){
    		buffer[n] = buffer[n];
    	}
    	buffer[n] = '\0'; 
    	if (n > 0) {
    		printf("Client : %s\n", buffer); 
    		sendto(sockfd, buffer, strlen(buffer), MSG_CONFIRM, 
    			(struct sockaddr *) &cliaddr, len); 
    	}else{
    		printf("%s\n", "ERROR receiving");
    	}
    }
}




int main() { 
	int socket_fd;
	struct sockaddr_in server_addr, client_addr;
	socklen_t sin_size;
	int bytes_read, len;
	char recv_data[1024];

	if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    	perror("UDP Server: ERROR while creating the socket.\n");
    	exit(1);
	}

	printf("Socket successfully created..\n");
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=INADDR_ANY;
    server_addr.sin_port=htons(PORT);
	printf("\nUDP Server: server socket binding...");
    printf("Hola\n");
	if (bind(socket_fd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) == -1) {
    	perror("UDP Server: ERROR while binding the socket.\n");
    	exit(1);
	}
	printf("\nUDP Server: done binding.");

	int pid; 
	int n, j;
	pid_t childpid;	
    procesoCliente(socket_fd, recv_data);
	/*while (1) {
    	printf("\nUDP Server: waiting for connection...\n");
    	bzero(recv_data,sizeof(recv_data));
    	int len;
   		bytes_read = recvfrom(socket_fd, recv_data, sizeof(recv_data), MSG_WAITALL, (struct sockaddr *) &client_addr, &len);
    	printf("\nConectando con %s:%d\n", inet_ntoa(client_addr.sin_addr),htons(client_addr.sin_port));
    	if (bytes_read > 0) {
    		n= 0;
    		for(; recv_data[n] != '\n'; n++){
    			recv_data[n] = recv_data[n];
    		}
    		recv_data[n] = '\0';
            printf("UDP Server: received -> %s\n", recv_data);
            pid = fork();

            if (pid < 0) {
                perror("UDP Server: ERROR while forking new process.\n");
                exit(1);

            }
            if (pid == 0) {
                procesoCliente(socket_fd, recv_data);
                close(socket_fd);
                exit(0);
            }else if (pid>0){
            	wait(NULL);
            }
        }
    }*/
	close(socket_fd);
	exit(0);
}






