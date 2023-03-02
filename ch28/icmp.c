#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ether.h>

int main(int argc, char **argv)
{
        //int fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
        int fd = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
        ETH_P_IP
        if(fd < 0){
                perror("socket");
                exit(0);
        }

        char buf[BUFSIZ] = {0};
        struct iphdr *iphdr;

        while(read(fd, buf, sizeof(buf))>0){
                printf("Read ICMP\n");
        }

        return 0;
}
