#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv)
{
	struct addrinfo hints, *result, *rp;
	int ret;

	bzero(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	if ((ret = getaddrinfo(argv[1], "5000", &hints, &result)) < 0) {
		perror(gai_strerror(ret));
		exit(0);
	}

	int fd, rlen;
	for (rp = result; rp; rp = rp->ai_next) {
		fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (fd < 0) {
			perror("socket");
			continue;
		}
		int on = 1;
		setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));
		break;
	}
	if (rp == NULL) {
		return 0;
	}

	if (argc > 2) {
		int len = strlen(argv[2]);
		if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, argv[2], len) < 0) {
			perror("setsockopt");
			exit(0);
		}
	}

	struct sockaddr_in peer;
	bzero(&peer, sizeof(peer));
	peer.sin_family = AF_INET;
	peer.sin_port = htons(8000);
        inet_pton(AF_INET, "0.0.0.0", &peer.sin_addr);
	if(bind(fd, &peer, sizeof(peer)) < 0){
		perror("bind");
		exit(0);
	}

	char buf[BUFSIZ] = { 0 };
	fd_set rset, trset;
	FD_ZERO(&rset);
	FD_SET(fd, &rset);
	FD_SET(0, &rset);

	for (;;) {
		trset = rset;
		if (select(fd + 1, &trset, NULL, NULL, NULL) < 0) {
			perror("select");
			continue;
		}

		if (FD_ISSET(0, &trset)) {
			rlen = read(0, buf, sizeof(buf));
			if (sendto(fd, buf, rlen, MSG_DONTROUTE, rp->ai_addr, rp->ai_addrlen) < 0) {
				perror("sendto");
				exit(0);
			}
		}

		if (FD_ISSET(fd, &trset)) {
			if ((rlen = read(fd, buf, sizeof(buf))) < 0) {
				perror("read");
				exit(0);
			}
			buf[rlen] = 0;
			printf("%s", buf);
		}
	}
}
