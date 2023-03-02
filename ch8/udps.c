#define _POSIX_C_SOURCE 200112L
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>

int main(int argc, char **argv)
{
	struct addrinfo ainfo;
        struct sockaddr_in localaddr, peeraddr;
        bzero(&localaddr, 0);

        localaddr.sin_family = AF_INET;
        localaddr.sin_port = htons(atoi(argv[2]));
        inet_pton(AF_INET, argv[1], &localaddr.sin_addr);

        int fd = socket(AF_INET, SOCK_DGRAM, 0);
	bind(fd, (void*)&localaddr, sizeof(localaddr));
        #if 0
        if(connect(fd, (void*)&localaddr, sizeof(localaddr)) < 0){
                perror("connect");
                exit(0);
        }
	#endif
	char buf[BUFSIZ] = { 0 };

	int len = sizeof(localaddr);

	for (;;) {
		if ((len=recvfrom(fd, buf, sizeof(buf), 0, (void *)&peeraddr, &len)) < 0) {
			perror("recvfrom");
			exit(0);
		}
		buf[len] = 0;
		char ip[INET_ADDRSTRLEN] = { 0 };
		inet_ntop(AF_INET, &peeraddr.sin_addr, ip, sizeof(ip));
		printf("Recv from:%s:%d<-%s\n", ip, ntohs(peeraddr.sin_port), buf);

		if (sendto(fd, buf, strlen(buf), 0, (void *)&peeraddr, sizeof(peeraddr)) < 0) {
			perror("sendto");
			exit(0);
		}
	}
}
