#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
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
        int fd, conn, len;
	if (argc < 2 || argc > 3) {
		printf("usage:daytime [<host>] <service or port>");
		exit(0);
	}

        struct addrinfo hints, *result, *rp;
        bzero(&hints, 0);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

	if (argc == 2) {
		getaddrinfo(NULL, argv[1], &hints, &result);
	} else {
		getaddrinfo(argv[1], argv[2], &hints, &result);
	}

	for (rp = result; rp; rp = rp->ai_next) {
		fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (fd < 0) {
			continue;
		}
                int on = 1;
                setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

                bind(fd, rp->ai_addr, rp->ai_addrlen);
                listen(fd, 5);
                freeaddrinfo(result);
                break;
	}

        //daemon(0, 0);
        for(;;){
                struct sockaddr_storage cli;
                len = sizeof(cli);
                conn = accept(fd, (void*)&cli, &len);
                char ip[INET6_ADDRSTRLEN], serv[8];
                getnameinfo((void*)&cli, len, ip, sizeof(ip), serv, sizeof(serv), 0);
                DBG("Client from:%s:%s", ip, serv);
                printf("Client from:%s:%s\n", ip, serv);

                time_t tm = time(NULL);
                char buf[BUFSIZ];
                snprintf(buf, BUFSIZ, "%s", ctime(&tm));
                if(write(conn, buf, strlen(buf)) < 0){
                        perror("write");
                        exit(0);
                }
                close(conn);
        }  
}