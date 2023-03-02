#define _POSIX_C_SOURCE 200112L
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>

int main(int argc, char **argv)
{
	struct addrinfo ainfo;
        struct sockaddr_in peeraddr, servaddr;
        bzero(&servaddr, 0);

        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(atoi(argv[2]));
        inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

        int fd = socket(AF_INET, SOCK_DGRAM, 0);
        #if 0
        if(connect(fd, (void*)&servaddr, sizeof(servaddr)) < 0){
                perror("connect");
                exit(0);
        }
        #endif
        char buf[BUFSIZ] = {0};

        int len = sizeof(servaddr);

        fd_set rfd, org_rfd;
        FD_ZERO(&org_rfd);
        FD_SET(0, &org_rfd);
	FD_SET(fd, &org_rfd);

	char ip[INET_ADDRSTRLEN] = { 0 };
	for(;;){
                rfd = org_rfd;
		select(fd + 1, &rfd, NULL, NULL, NULL);
		if (FD_ISSET(0, &rfd)) {
			len = read(0, buf, sizeof(buf));
                        buf[len] = 0;
                        inet_ntop(AF_INET, &servaddr.sin_addr, ip, sizeof(ip));
                        printf("Send to %s:%d->%s\n", ip, ntohs(servaddr.sin_port), buf);
			if (sendto(fd, buf, len, 0, (void *)&servaddr, sizeof(servaddr)) < 0) {
				perror("sendto");
				exit(0);
			}
		}

		if (FD_ISSET(fd, &rfd)) {
                        len = sizeof(peeraddr);
			if ((len=recvfrom(fd, buf, sizeof(buf), 0, (void *)&peeraddr, &len)) < 0) {
				perror("recvfrom");
				exit(0);
			}
                        buf[len] = 0;
			inet_ntop(AF_INET, &peeraddr.sin_addr, ip, sizeof(ip));
			printf("Recv from:%s:%d<-%s\n", ip, ntohs(peeraddr.sin_port), buf);
		}
	}
        return 0;
}
