COMPILER = gcc

server: udpServer.c
	${COMPILER} -o server udpServer.c
client: udpClient.c
	${COMPILER} -o client udpClient.c