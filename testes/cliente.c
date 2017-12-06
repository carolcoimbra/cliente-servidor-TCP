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
CLIENTE:
processa argumentos da linha de comando:
	host_do_servidor (argv[1]) -> tipo char*
	porta_servidor (argv[2]) -> tipo int
	nome_arquivo (argv[3]) -> tipo char*
	tam_buffer (argv[4]) -> tipo int
chama gettimeofday para tempo inicial
faz abertura ativa a host_do_servidor : porta_servidor
envia string com nome do arquivo (terminada em zero)
abre arquivo que vai ser gravado - pode ser fopen(nome,"w+")
loop recv buffer até que receba zero bytes ou valor negativo
	escreve bytes do buffer no arquivo (fwrite)
	atualiza contagem de bytes recebidos
fim_loop
fecha conexão e arquivo
chama gettimeofday para tempo final e calcula tempo gasto
imprime resultado:
"Buffer = \%5u byte(s), \%10.2f kbps (\%u bytes em \%3u.\%06u s)
*/

int main (int argc, char** argv){

	if (argc != 5)
		error("Erro! Quantidade de argumentos errada!\n", -1, -1);
	
	int client_socket, server_socket, ip_type;
	int i, t, code, bytes_r, t1, t2;
	struct timeval start, finish;

	//Recebimento dos parametros da linha de comando
	char *ip = argv[1];
	int port = atoi(argv[2]);
	char *file_name = argv[3];
	int buffer_len = atoi(argv[4]);

	char *buffer = (char*)malloc(sizeof(char)*buffer_len);
	char *msg_read = (char*)malloc(sizeof(char)*200);
	char *file_out = (char*)malloc(sizeof(char)*100);

	//Importante: é necessário distinguir entre o IPv4 e o IPv6
	ip_type = selectIP (ip);

	if (ip_type == 4){
		//printf("IPv4\n");
		struct sockaddr_in server_address;
		bzero(&server_address,sizeof(server_address));	
		server_address.sin_family = AF_INET;
		server_address.sin_port = htons(port);
		inet_aton(ip, &server_address.sin_addr.s_addr);

		//Criação de um socket: int socket(int domain, int type, int protocol);
		//printf("Criando socket cliente...\n");
		client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (client_socket < 0)
			error("Erro ao criar o socket do cliente!\n", -1, -1);


		//Abertura ativa: int connect(int socket, struct socket_address *addr, int addr_len)
		printf("Conectando com o servidor...\n");
		server_socket = connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address));
		if (server_socket < 0)
			error("Erro ao conectar com o socket do servidor!\n", -1, client_socket);

		printf("Conexão estabelecida!\n");
	}

	else if (ip_type == 6){
		//printf ("IPv6\n");
		struct sockaddr_in6 server_address;
		bzero(&server_address,sizeof(server_address));	
		server_address.sin6_family = AF_INET6;
		server_address.sin6_port = htons(port);
		inet_aton(ip, &server_address.sin6_addr.s6_addr);

		//Criação de um socket: int socket(int domain, int type, int protocol);
		//printf("Criando socket cliente...\n");
		client_socket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
		if (client_socket < 0)
			error("Erro ao criar o socket do cliente!\n", -1, -1);

		//Abertura ativa: int connect(int socket, struct socket_address *addr, int addr_len)
		printf("Conectando com o servidor...\n");
		server_socket = connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address));
		if (server_socket < 0)
			error("Erro ao conectar com o socket do servidor (%d)!\n", -1, client_socket);

		printf("Conexão estabelecida!\n");
	}

	else
		error("Erro ao identificar o IP! (Codigo %d)\n", -1, -1);

	for (t=0; t<10; t++){
		bytes_r = 0;

		//Chama gettimeofday para tempo inicial
		gettimeofday(&start, NULL);

		//Envio de mensagens: READY será a primeira mensagem!
		//int send(int socket, char *msg, int msg_len, int flags)
		//printf("Enviando READY...\n");
		code = send(client_socket, "READY", 6, 0);
		if(code < 0)
			error("Erro ao enviar READY!\n", server_socket, client_socket);

		code = recv(client_socket, msg_read, 100, 0);
		if(code < 0 || strcmp(msg_read, "READY_ACK") != 0)
			error("Erro ao receber READY_ACK!\n", server_socket, client_socket);

		//bytes_r += code;

		//Envio de mensagens: O nome do arquivo será a segunda mensagem!
		//int send(int socket, char *msg, int msg_len, int flags)
		//printf("Enviando nome do arquivo...\n");
		for (i=0; i<strlen(file_name); i++){
			//printf ("Enviando... %c\n", file_name[i]);
			code = send(client_socket, &file_name[i], 1, 0);
			if(code < 0)
				error("Erro ao enviar nome do arquivo!\n", server_socket, client_socket);
		}

		code = send(client_socket, "\0", 1, 0);
		if(code < 0)
			error("Erro ao enviar EOF!\n", server_socket, client_socket);

		code = recv(client_socket, msg_read, 100, 0);
		if(code < 0 || strcmp(msg_read, "FILE_ACK") != 0)
			error("Erro ao receber FILE_ACK!\n", server_socket, client_socket);

		//bytes_r += code;

		//abre arquivo que vai ser gravado - pode ser fopen(nome,"w+")
		sprintf (file_out, "%s.out", file_name);
		FILE *file_write = fopen(file_out, "a");
	
		//Recebimento do arquivo em buffers
		//loop recv buffer até que receba zero bytes ou valor negativo
		while(1){
		    code = recv(client_socket, buffer, buffer_len, 0);
			if (strcmp(buffer, "\0") == 0){
				break;
			}

			if (code < 0)
				error("Erro ao receber buffer!\n", server_socket, client_socket);

			//escreve bytes do buffer no arquivo (fwrite)
			//size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
			//atualiza contagem de bytes recebidos
			//printf("%s\n", buffer);
			fprintf(file_write, buffer);
			bytes_r += code;

			/*
			code = send(client_socket, "ACK", 4, 0);
			if (code < 0)
				error("Erro ao enviar ACK!\n", server_socket, client_socket);
			*/

		}

		//printf("Enviando BYE...\n");
		code = send(client_socket, "BYE", 4, 0);
		if(code < 0)
			error("Erro ao enviar BYE!\n", server_socket, client_socket);

		code = recv(client_socket, msg_read, 100, 0);
		if(code < 0 || strcmp(msg_read, "BYE_ACK") != 0)
			error("Erro ao receber BYE_ACK!\n", server_socket, client_socket);

		//bytes_r += code;

		//Fechando o arquivo
		fclose (file_write);

		//Chama gettimeofday para tempo final e calcula tempo gasto
		gettimeofday(&finish, NULL);

		t1 = (start.tv_sec*1000000 + start.tv_usec);
		t2 = (finish.tv_sec*1000000 + finish.tv_usec);

		sprintf (file_out, "estatisticas%d", buffer_len);
		FILE *file = fopen(file_out, "a");
		fprintf(file, "%d,%d,%d\n", buffer_len, bytes_r, t2-t1);
		fclose(file);
	
		//Imprime resultado: "Buffer = \%5u byte(s), \%10.2f kbps (\%u bytes em \%3u.\%06u s)
		printf("Tempo gasto: %d usec\n", t2-t1);
		printf("Quantidade de dados recebidos: %d bytes\n", bytes_r);
	}

	//Fechamento: int close (int socket)
	close (client_socket);
	close (server_socket);

	free (buffer);
	free (msg_read);
	free(file_out);
	
	return 0;
}
