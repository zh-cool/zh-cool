#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv)
{
        struct addrinfo hints, *result, *rp;
	int ret, fd, rlen, on;

	bzero(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	if ((ret = getaddrinfo(argv[1], "5000", &hints, &result)) < 0) {
		perror(gai_strerror(ret));
		exit(0);
	}
	for (rp = result; rp; rp = rp->ai_next) {
		fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (fd < 0) {
			perror("socket");
			continue;
		}
		break;
	}
        if(rp == NULL){
                printf("NO INT");
                exit(0);
        }
        if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0){
                perror("SO_REUSEADDR");
                exit(0);
        }

        if(bind(fd, rp->ai_addr, rp->ai_addrlen) < 0){
                perror("bind rp");
                exit(0);
        }
        
        ret = 0;
        struct group_req greq;
        int level = rp->ai_family == AF_INET ? IPPROTO_IP : IPPROTO_IPV6;
        greq.gr_group = *(struct sockaddr_storage*)rp->ai_addr; 
        greq.gr_interface = if_nametoindex("eth0");
        ret += setsockopt(fd, level, MCAST_JOIN_GROUP, &greq, sizeof(greq));
        /*
        greq.gr_interface = if_nametoindex("eth0.13");
        ret += setsockopt(fd, level, MCAST_JOIN_GROUP, &greq, sizeof(greq));
        greq.gr_interface = if_nametoindex("dummy0");
        ret += setsockopt(fd, level, MCAST_JOIN_GROUP, &greq, sizeof(greq));
        */
        if(ret != 0){
                perror("MCAST_JOIN_GROUP");
                exit(0);
        }

        for(;;){
                char buf[BUFSIZ] = {0};
                rlen = read(fd, buf, sizeof(buf));
                if(rlen < 0){
                        perror("read");
                        exit(0);
                }
                buf[rlen] = 0;
                fputs(buf, stdout);
        }
        
        return 0;
}
