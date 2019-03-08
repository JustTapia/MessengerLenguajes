#include <stdlib.h>
#include <stdio.h>
#include "Usuario.h"
#ifndef REGISTRARUSUARIO
#define REGISTRARUSUARIO
	void registrarUsuario(char nombre[], char ip[], int puerto);
#endif

typedef struct Usuario usuario;

void registrarUsuario(char nombre[], char ip[], int puerto){
	struct usuario{
		int puerto;
		char *usuario;
		char *ip;
	};
	printf("Registro completado!\n");
}