#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <setjmp.h>

jmp_buf reset;

void sig_bus(int sig)
{
	//longjmp(reset, 1);
}

        char buf[BUFSIZ];
int main(int argc, char **argv)
{
	//signal(SIGBUS, sig_bus);
	//signal(SIGSEGV, sig_bus);

	struct addrinfo hints, *result, *rp;
	bzero(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int ret;
	char *host = NULL;
	char *service = "5000";
	if (argc > 2) {
		host = argv[1];
		service = argv[2];
	} else {
		service = argv[1];
	}
	if ((ret = getaddrinfo(host, service, &hints, &result)) < 0) {
		perror(gai_strerror(ret));
		exit(0);
	}

        char ip[INET6_ADDRSTRLEN] = { 0 };
        char serv[8] = { 0 };
        int fd;
        for (rp = result; rp; rp = rp->ai_next) {
                ret = getnameinfo(rp->ai_addr, rp->ai_addrlen, ip, sizeof(ip), serv, sizeof(serv),
                                NI_NUMERICHOST | NI_NUMERICSERV);
                if (ret < 0) {
                        perror(gai_strerror(ret));
                        exit(0);
                }

                fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
                if (fd < 0) {
                        perror("socket");
                        exit(0);
                }
                int on = 1;
                setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
                if (bind(fd, rp->ai_addr, rp->ai_addrlen) < 0) {
                        perror("bind");
                        exit(0);
                }
                listen(fd, 5);

                printf("Serv:%s:%s\n", ip, serv);
                break;
        }


        sleep(5);
        for (;;) {
                struct sockaddr_storage peer;
                socklen_t len = sizeof(peer);
                printf("begin 11 accept\n");
		int conn = accept(fd, (void *)&peer, &len);
		if (conn < 0) {
			perror("accept");
			exit(0);
		}
                getnameinfo((void*)&peer, len, ip, sizeof(ip), serv, sizeof(serv), NI_NUMERICHOST|NI_NUMERICSERV);
                printf("Client:%s:%s\n", ip, serv);


                if(getpeername(conn, (void*)&peer, &len) < 0){
                        perror("getpeername");
                }

		if ((len = read(conn, buf, 10)) < 0) {
			perror("read");
			exit(0);
		} else {
			buf[len] = 0;
			printf("read: %s %d\n", buf, len);
		}
		printf("read after:%s %d\n", buf, len);
	}

	return 0;
}
