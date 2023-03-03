#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ether.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/select.h>


int send_v4(int id, int seq)
{
        struct icmphdr icmp;
}

void run_loop(int fd, struct addrinfo *peer)
{
        struct timeval tv;
        fd_set rset, trset;
        FD_ZERO(&rset);
        FD_SET(fd, &rset);
        tv.tv_sec=1;
        tv.tv_usec = 0;

        int num = 0;
        for(;;){
                trset = rset;
                num = select(fd+1, &trset, 0, 0, &tv);
                if(num < 0){
                        perror("select");
                        exit(0);
                }
                if(num == 0){
                        printf("send icmp\n");
                        tv.tv_sec = 1;
                        tv.tv_usec = 0;
                        continue;
                }
                int ret;
                struct sockaddr_storage cli;
	        char ip[INET6_ADDRSTRLEN], serv[8], buf[1024];
                socklen_t len = sizeof(cli);
                if (recvfrom(fd, buf, sizeof(buf), 0, (void*)&cli, &len) > 0) {
                        if ((ret = getnameinfo((void*)&cli, len, ip, sizeof(ip), serv, sizeof(serv), NI_NUMERICHOST | NI_NUMERICSERV)) < 0) {
                                perror(gai_strerror(ret));
                                exit(0);
                        }
                        printf("from:%s:%s family:%d\n", ip, serv, cli.ss_family);
                }

        }
}

int main(int argc, char **argv)
{
	char buf[BUFSIZ] = { 0 };
	int ret, fd;

	struct addrinfo hints, *result, *rp;
	bzero(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_RAW;
	//hints.ai_protocol = IPPROTO_ICMP;
	char *host = NULL;
	if (argc <= 3) {
		host = argv[1];
	}

	if ((ret = getaddrinfo(host, NULL, &hints, &result)) < 0) {
		perror(gai_strerror(ret));
		exit(0);
	}

	char ip[INET6_ADDRSTRLEN], serv[8];
	for (rp = result; rp; rp = rp->ai_next) {
		if ((ret = getnameinfo(rp->ai_addr, rp->ai_addrlen, ip, sizeof(ip), serv, sizeof(serv), NI_NUMERICHOST | NI_NUMERICSERV)) < 0) {
			perror(gai_strerror(ret));
			exit(0);
		}
		printf("%s:%s type:%d proto:%d family:%d\n", ip, serv, rp->ai_socktype, rp->ai_protocol, rp->ai_family);

		int proto = rp->ai_family == AF_INET ? IPPROTO_ICMP : IPPROTO_ICMPV6;
		if ((fd = socket(rp->ai_family, rp->ai_socktype, proto)) < 0) {
			perror("socket");
			continue;
		}
#if 0
		if(bind(fd, rp->ai_addr, rp->ai_addrlen) < 0){
			perror("bind");
			close(fd);
			continue;
		}
		struct sockaddr_in cli;
		bzero(&cli, sizeof(cli));
		cli.sin_family = AF_INET;
		cli.sin_port = 0;
		if(inet_pton(AF_INET, argv[2], &cli.sin_addr) < 0){
			perror("inet_pton");
			exit(0);
		}
		if (connect(fd, &cli, sizeof(cli)) < 0) {
			if (errno == EAFNOSUPPORT) {
				perror("connect");
				exit(0);
			}
		}
#endif
		break;
	}
	if (!rp) {
		return 0;
	}

        run_loop(fd, rp);
#if 0
	struct sockaddr_in cli;
	bzero(&cli, sizeof(cli));
	socklen_t len = sizeof(cli);
	while (recvfrom(fd, buf, sizeof(buf), 0, &cli, &len) > 0) {
		if ((ret = getnameinfo((void*)&cli, len, ip, sizeof(ip), serv, sizeof(serv), NI_NUMERICHOST | NI_NUMERICSERV)) < 0) {
			perror(gai_strerror(ret));
			exit(0);
		}
		printf("from:%s:%s family:%d\n", ip, serv, cli.sin_family);
	}
#endif
	return 0;
}
