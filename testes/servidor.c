/*

UNIVERSIDADE FEDERAL DE MINAS GERAIS (UFMG)
INSTITUTO DE CIÊNCIAS EXATAS (ICEX)
DEPARTAMENTO DE CIÊNCIA DA COMPUTAÇÃO (DCC)

		REDES DE COMPUTADORES 
		  TRABALHO PRÁTICO 1

CAROLINA COIMBRA VIEIRA - 2014032941

*/

#include <sys/time.h> //gettimeofday()
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tp1.h"

/*
SERVIDOR:
processa argumentos da linha de comando:
	porta_servidor (argv[1]) -> tipo int
	tam_buffer (argv[2]) -> tipo int
faz abertura passiva e aguarda conexão
recebe, byte a byte até o zero, o string com nome do arquivo
abre arquivo que vai ser lido -- pode ser fopen(nome,"r")
se deu erro, fecha conexão e termina
loop lê o arquivo, um buffer por vez até fread retornar zero
	envia o buffer lido
	se quiser, contabiliza bytes enviados
fim_loop
fecha conexão e arquivo
chama gettimeofday para tempo final e calcula tempo gasto
se quiser, imprime nome arquivo e no. de bytes enviados
fim_servidor.
*/

int main (int argc, char** argv){

	if (argc != 3)
		error("Erro! Quantidade de argumentos errada!\n", -1, -1);

	int client_socket, server_socket;
	int i, t, code, bytes_s, t1, t2;
	struct timeval start, finish;

	//Recebimento dos parametros da linha de comando
	int port = atoi(argv[1]);
	int buffer_len = atoi(argv[2]);

	char *buffer = (char*)malloc(sizeof(char)*buffer_len);
	char *msg_read = (char*)malloc(sizeof(char)*200);

	// O servidor IPV6 aceita tanto IPV4 quanto IPV6
	struct sockaddr_in6 server_address, client_address;
	bzero(&server_address,sizeof(server_address));	
	server_address.sin6_family = AF_INET6;
   	server_address.sin6_port = htons(port);
	server_address.sin6_addr = in6addr_any;

	int client_len = sizeof(client_address);

	//Criação de um socket: int socket(int domain, int type, int protocol);	
	server_socket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
	if (server_socket < 0)
		error("Erro ao criar o socket do servidor!\n", -1, -1);

	//Abertura passiva: int bind(int socket, struct socket_address *addr, int addr_len)
	//Atribui um endereço IP e uma porta a um socket 
	code = bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address));
	if(code < 0)
		error("Erro bind!\n", server_socket, -1);

	code = listen(server_socket, 10);
	if (code < 0)
		error("Erro no listen!\n", server_socket, -1);

	//Aguarda a conexão
	printf("Esperando conexão...\n");
	client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_len);
	if (client_socket < 0)
		error("Erro no accept!\n", server_socket, -1);

	printf("Conexão estabelecida!\n");

	for (t=0; t<10; t++){
		bytes_s = 0;

		//Chama gettimeofday para tempo inicial
		gettimeofday(&start, NULL);

		//Recebimento de mensagens: READY será a primeira mensagem!
		//recv(int socket, char *buffer, int buffer_len, int flags)
		code = recv(client_socket, msg_read, 100, 0);
		if(code < 0 || strcmp(msg_read, "READY") != 0)
			error("Erro ao receber READY!\n", server_socket, client_socket); 

		//printf("Enviando READY_ACK...\n");
		code = send(client_socket, "READY_ACK", 10, 0);
		if(code < 0)
			error("Erro ao enviar READY_ACK!\n", server_socket, client_socket);

		bytes_s += code;
		i = 0;

		//Recebimento de mensagens: O nome do arquivo será a segunda mensagem!
		//recv(int socket, char *buffer, int buffer_len, int flags)
		//recebe, byte a byte até o zero, o string com nome do arquivo
		while (1){
			code = recv(client_socket, &msg_read[i], 1, 0);	
			//printf("Recebendo... %c\n", msg_read[i]);		

			if (msg_read[i] == '\0')
				break;

			if (code < 0)
				error("Erro ao receber nome do arquivo!\n", server_socket, client_socket);

			i++;
		}
		msg_read[i] = '\0';

		//printf("Enviando FILE_ACK...\n");
		code = send(client_socket, "FILE_ACK", 10, 0);
		if(code < 0)
			error("Erro ao enviar FILE_ACK!\n", server_socket, client_socket);

		bytes_s += code;

		//printf("Arquivo recebido: %s\n", msg_read);
		//abre arquivo que vai ser lido -- pode ser fopen(nome,"r")
		//se deu erro, fecha conexão e termina
		FILE *file_read = fopen(msg_read, "r");
		if (file_read == NULL) 
		    error("Erro ao abrir o arquivo!\n", server_socket, client_socket);


		//char *fgets(char *str, int n, FILE *stream)
		while (fgets(buffer, buffer_len, file_read) != NULL){
			//printf("%s", buffer);	
	
			code = send(client_socket, buffer, buffer_len, 0);
			if (code < 0)
				error("Erro ao enviar buffer!\n", server_socket, client_socket);		

			bytes_s += code;

			/*
			code = recv(client_socket, msg_read, 100, 0);
			if (code < 0)
				error("Erro ao receber ACK!\n", server_socket, client_socket);		
			*/	
		}

		//Envia EOF bytes para o cliente saber que acabou
		code = send(client_socket, "\0", 1, 0);
		if (code < 0)
			error("Erro ao enviar EOF!\n", server_socket, client_socket);		

		bytes_s += code;

		code = recv(client_socket, msg_read, 100, 0);
		if (code < 0 || strcmp(msg_read, "BYE") != 0)
			error("Erro ao receber BYE!", server_socket, client_socket);

		//printf("Enviando BYE_ACK...\n");
		code = send(client_socket, "BYE_ACK", 10, 0);
		if(code < 0)
			error("Erro ao enviar BYE_ACK!\n", server_socket, client_socket);

		bytes_s += code;

		//Fechando o arquivo
		fclose(file_read);

		//Chama gettimeofday para tempo final e calcula tempo gasto
		gettimeofday(&finish, NULL);

		t1 = (start.tv_sec*1000000 + start.tv_usec);
		t2 = (finish.tv_sec*1000000 + finish.tv_usec);

		//Imprime resultado: "Buffer = \%5u byte(s), \%10.2f kbps (\%u bytes em \%3u.\%06u s)
		printf("Tempo gasto: %d usec\n", t2-t1);
		printf("Quantidade de dados enviados: %d bytes\n", bytes_s);
	}
	
	//Fechamento: int close (int socket)
	close(server_socket);
	close(client_socket);
	//printf("O cliente se desconectou!\n");

	free (buffer);
	free (msg_read);

	return 0;
}
