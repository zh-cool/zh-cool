#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

void sig_handle(int sig)
{
	int status;
	wait(&status);
	return;
}

int str_echo(int connfd)
{
	char buf[1024], ip[INET_ADDRSTRLEN];
	struct sockaddr_in peeraddr;
	int len = sizeof(peeraddr);
	if(getpeername(connfd, (void*)&peeraddr, &len) < 0){
		perror("getpeername");
		exit(0);
	}
	inet_ntop(AF_INET, &peeraddr.sin_addr, ip, sizeof(ip));
	printf("Client from:%s:%d\n", ip, ntohs(peeraddr.sin_port));
	while((len=read(connfd, buf, sizeof(buf))) > 0){
		buf[len] = 0;
		printf("recv:%s from %s %d\n", buf, ip, ntohs(peeraddr.sin_port));
		write(connfd, buf, strlen(buf));
	}
	printf("Client %s:%d leave\n", ip, ntohs(peeraddr.sin_port));
	close(connfd);
	return 0;
}

int main(int argc, char **argv)
{
	struct sockaddr_in servaddr, cliaddr;
	int listenfd, connfd;

	signal(SIGCHLD, sig_handle);
	if((listenfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		perror("socket");
		exit(0);
	}
	int on = 1; 
	if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on))){
		perror("setsockopt");
		exit(0);
	}

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	if(argv[1]){
		inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
	}else{
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	servaddr.sin_port  = htons(5000);
	if(bind(listenfd, (void*)&servaddr, sizeof(servaddr)) < 0){
		perror("bind");
		exit(0);
	}

	listen(listenfd, 5);

	for (;;) {
		char buf[BUFSIZ];
		socklen_t len = sizeof(cliaddr);
		if(recvfrom(listenfd, buf, sizeof(buf), 0, (void*)&cliaddr, &len) < 0){
			perror("recvfrom");
			exit(0);
		}		
		printf("Recv %s to %d\n", buf, getpid());
	}
	return 0;
}