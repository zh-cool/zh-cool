#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <net/if.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv)
{
        int fd, ret, rlen;
        struct addrinfo hints, *result, *rp;
        bzero(&hints, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;
        if((ret=getaddrinfo(argv[1], argv[2], &hints, &result)) < 0){
                perror(gai_strerror(ret));
                exit(0);
        }
        for(rp=result; rp; rp=rp->ai_next){
                fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
                if(fd < 0){
                        perror("sock");
                        exit(0);
                }
                break;
        }
        if(rp == NULL){
                printf("no sock\n");
                exit(0);
        }

        int on=1;
        if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))<0){
                perror("SO_REUSEADDR");
                exit(0);
        }
        if(bind(fd, rp->ai_addr, rp->ai_addrlen) < 0){
                perror("bind");
                exit(0);
        }
/*
	if (argc > 3) {
		if (rp->ai_family == AF_INET) {
			struct ip_mreqn mreq;
			mreq.imr_ifindex = if_nametoindex(argv[3]);
			mreq.imr_multiaddr = ((struct sockaddr_in *)rp->ai_addr)->sin_addr;
			if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, &mreq, sizeof(mreq)) < 0) {
				perror("IP_MULTICAST_IF");
				exit(0);
			}
		} else {
			struct ipv6_mreq mreq;
			mreq.ipv6mr_interface = if_nametoindex(argv[3]);
			mreq.ipv6mr_multiaddr = ((struct sockaddr_in6 *)rp->ai_addr)->sin6_addr;
			if (setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_IF, &mreq, sizeof(mreq)) < 0) {
				perror("IP_MULTICAST_IF");
				exit(0);
			}
		}
	}
*/

        struct group_req greq;
        greq.gr_group = *(struct sockaddr_storage*)rp->ai_addr;
        greq.gr_interface = if_nametoindex("eth0");
        int level = rp->ai_family==AF_INET ? IPPROTO_IP : IPPROTO_IPV6;
        if(setsockopt(fd, level, MCAST_JOIN_GROUP, &greq, sizeof(greq)) < 0){
                perror("MCAST_JOIN_GROUP");
                exit(0);
        }
        greq.gr_interface = if_nametoindex("eth0.13");
        if(setsockopt(fd, level, MCAST_JOIN_GROUP, &greq, sizeof(greq)) < 0){
                perror("MCAST_JOIN_GROUP");
                exit(0);
        }

        for(;;){
                char buf[1024];
                if((rlen=read(fd, buf, sizeof(buf))) < 0){
                        perror("read");
                        exit(0);
                }
                buf[rlen] = 0;
                printf("%s", buf);
        }

}
