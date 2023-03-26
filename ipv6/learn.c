#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <byteswap.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
        struct sockaddr_in6 addr;
	int s;

	if ((s = inet_pton(AF_INET6, "192.168.1.1", &addr.sin6_addr)) != 1) {
		if (s == 0) {
			fprintf(stderr, "Not in presentation format");
		} else {
			perror("inet_pton");
		}
		exit(0);
	}
	printf("%04x", addr.sin6_addr.s6_addr16[0]);
        return 0;
}