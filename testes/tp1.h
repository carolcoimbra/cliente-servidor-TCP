#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void error(char *info, int server_socket, int client_socket){

	printf("%s\n", info);
	
	if(client_socket != -1){
		close(server_socket);
	}

	if(server_socket != -1){
		close(client_socket);
	}

	exit(1);
}

int selectIP(char *ip){
	struct addrinfo hint, *ip_type = NULL;
	int code, ip_number;
	memset(&hint, '\0', sizeof hint);

	hint.ai_family = PF_UNSPEC;
	hint.ai_flags = AI_NUMERICHOST;
	
	//Salva em ip_type o tipo de IP (IPv4 ou IPv6)
	// getaddrinfo() returns 0 if it succeeds
	code = getaddrinfo(ip, NULL, &hint, &ip_type);
	if (code != 0) {
		ip_number = -1;
	}
	if(ip_type->ai_family == AF_INET) {
		ip_number = 4;
	}
	else if (ip_type->ai_family == AF_INET6) {
		ip_number = 6;
    } 
	else {
		ip_number = 1;
	}
	
	freeaddrinfo(ip_type);
	return ip_number;
}


