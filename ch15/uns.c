//#define _GNU_SOURCE 
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>

#define SOCK_NAME "/tmp/austin.sock"
int main(int argc, char **argv)
{
        struct sockaddr_un un;
        int len = sizeof(un);

        unlink(SOCK_NAME);
        un.sun_family = AF_UNIX;
        strncpy(un.sun_path, SOCK_NAME, sizeof(un.sun_path));
        printf("SUN_LEN:%ld sizeof:%ld\n", SUN_LEN(&un), sizeof(un));

        int sfd = socket(AF_UNIX, SOCK_STREAM, 0);
        if(bind(sfd, (void*)&un, len) < 0){
                perror("bind");
                exit(0);
        }
        listen(sfd, 5);

        bzero(&un, sizeof(un));
        len = sizeof(un);
        int conn = accept(sfd, (void*)&un, &len);
        if(conn < 0){
                perror("accept");
                exit(0);
        }

        int fd = open("/home/austin/Src/learn/ch5/tcserver.c", O_RDWR);                
        if(fd < 0){
                perror("OPEN");
                exit(0);
        }

        char buf[1024];
        struct msghdr msg;
        msg.msg_controllen = CMSG_SPACE(sizeof(int));
        msg.msg_control = buf;

        struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
        cmsg->cmsg_len = CMSG_LEN(sizeof(int));
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;
        int *ptr = (void*)CMSG_DATA(cmsg);
        *ptr = fd;

        struct iovec iov[1];
        iov[0].iov_base = malloc(1024);
        iov[0].iov_len = strlen("austin");
        strcpy(iov[0].iov_base, "austin");

        msg.msg_iov = iov;
        msg.msg_iovlen = 1;
        msg.msg_name = NULL;
        msg.msg_namelen = 0;

        if((len=sendmsg(conn, &msg, 0)) < 0){
                perror("sendmsg");
                exit(0);
        }
        printf("Send:%d\n", len);
        pause();
        return 0;
}