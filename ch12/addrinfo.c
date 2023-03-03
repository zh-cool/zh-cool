#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ether.h>
#include <netdb.h>
#include <syslog.h>
#include <time.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#define DBG(M, ...) syslog(LOG_INFO|LOG_LOCAL0, M, ##__VA_ARGS__)
int main(int argc, char **argv)
{
        int fd, conn, len, ret;
	if (argc < 2 || argc > 3) {
		printf("usage:daytime [<host>] <service or port>");
		exit(0);
	}

        struct addrinfo hints, *result, *rp;
        bzero(&hints, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        //hints.ai_socktype = SOCK_STREAM;
        //hints.ai_socktype = SOCK_RAW;
        hints.ai_flags = AI_PASSIVE;
        //hints.ai_protocol = IPPROTO_TCP;
        ETH_P_IP

	if (argc == 2) {
		ret = getaddrinfo(NULL, argv[1], &hints, &result);
	} else {
		ret = getaddrinfo(argv[1], argv[2], &hints, &result);
	}
        if(ret){
                printf("getaddrinfo %s\n", gai_strerror(ret));
                exit(0);
        }

	for (rp = result; rp; rp = rp->ai_next) {
                char ip[INET6_ADDRSTRLEN+1024], serv[32];
                if((ret=getnameinfo(rp->ai_addr, rp->ai_addrlen, ip, sizeof(ip), serv, sizeof(serv), NI_NUMERICHOST|NI_NUMERICSERV)) < 0){
                        printf("getnameinfo %s %d\n", gai_strerror(ret), ret);
                        if(ret == EAI_AGAIN){
                                printf("again");
                        }
                }
                printf("Client <- %s:%s %d %d\n", ip, serv, rp->ai_socktype, rp->ai_protocol);
	}
        freeaddrinfo(result);

        return 0;
}
