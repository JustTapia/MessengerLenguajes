#include <stdlib.h>
#include <stdio.h>
#include "Usuario.h"
#ifndef REGISTRARUSUARIO
#define REGISTRARUSUARIO
	void registrarUsuario(char nombre[], char ip[], int puerto);
#endif


int main(void){
	char tmp, ip[15], sPuerto[2], nombre[30];
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
	int puerto = atoi(sPuerto);
	registrarUsuario(nombre,ip,puerto);
	return 0;
}