//#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>

int main(int argc, char **argv)
{
        int ret;
        int sk;
        struct addrinfo hints, *result, *rp;

        bzero(&hints, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_flags = AI_V4MAPPED|AI_PASSIVE;

        if((ret=getaddrinfo(argv[1], argv[2], &hints, &result))<0){
                perror(gai_strerror(ret));
                exit(0);
        }

        char ip[INET6_ADDRSTRLEN], serv[8];
        for(rp=result; rp; rp=rp->ai_next){
                if(ret=getnameinfo(rp->ai_addr, rp->ai_addrlen, ip, sizeof(ip), serv, sizeof(serv), NI_NUMERICHOST|NI_NUMERICSERV) < 0){
                        perror(gai_strerror(ret));
                        exit(0);
                }
                sk = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
                if(bind(sk, rp->ai_addr, rp->ai_addrlen) < 0){
                        perror("bind");
                        exit(0);
                }
                syslog(LOG_INFO|LOG_LOCAL0, "[%d]Address: [%s]:[%s]", getpid(), ip, serv);
                break;
        }

        for(;;){
                char buf[1024];
                struct sockaddr_storage cli;
                int len = sizeof(cli); 
                len = recvfrom(sk, buf, sizeof(buf), MSG_PEEK, (void*)&cli, &len); 

                int value;
                ret = ioctl(sk, FIONREAD, &value);
                printf("Rcvfrom len:%d ioctl:%d\n", len, value);
                len = recvfrom(sk, buf, sizeof(buf), 0, (void*)&cli, &len); 
        }

        return 0;
}