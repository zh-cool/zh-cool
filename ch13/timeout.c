#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h>
#include <sys/uio.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void sig_timeout(int sig)
{
        sig = __IOV_MAX;
        printf("TIME OUT\n");
        return;
}
int main(int argc, char **argv)
{
        struct addrinfo hints, *result, *rp;
        signal(SIGALRM, sig_timeout);

        bzero(&hints, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = 0;

        int ret = getaddrinfo(argv[1], argv[2], &hints, &result);
        if(ret){
		printf("getaddrinfo:%s\n", gai_strerror(ret));
	}

	for (rp = result; rp; rp = rp->ai_next) {
		char ip[INET6_ADDRSTRLEN] = { 0 }, serv[8] = { 0 };
		getnameinfo(rp->ai_addr, rp->ai_addrlen, ip, sizeof(ip), serv, sizeof(serv), 0);
                printf("Connect to:%s:%s\n", ip, serv);

		int fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
                struct timeval tv;
                tv.tv_sec = 2;
                tv.tv_usec = 0;
                setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
                setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
                if(connect(fd, rp->ai_addr, rp->ai_addrlen) < 0){
                        perror("connect");
                        exit(0);
                }

                char buf[1204];
                strncpy(buf, "Hello World!", sizeof(buf));
                if(send(fd, buf, strlen(buf), MSG_DONTWAIT) < 0){
                        perror("send");
                        exit(0);
                }
                int flags = 0;//MSG_DONTWAIT;
                if(recv(fd, buf, sizeof(buf), flags) < 0){
                        if(errno == EAGAIN){
                                printf("EAGAIN\n");
                        }
                        if(errno == EWOULDBLOCK){
                                printf("EWOULDBLOCK\n");
                        }
                        perror("recv");
                        exit(0);
                }
                printf("Exit");
	}
	return 0;
}