//#define _GNU_SOURCE 
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

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

        un.sun_family = AF_UNIX;
        strncpy(un.sun_path, SOCK_NAME, sizeof(un.sun_path));
        printf("SUN_LEN:%ld sizeof:%ld\n", SUN_LEN(&un), sizeof(un));

        int sfd = socket(AF_UNIX, SOCK_STREAM, 0);
        /*
        if(bind(sfd, (void*)&un, SUN_LEN(&un)) < 0){
                perror("bind");
                exit(0);
        }
        if(bind(sfd, (void*)&un, sizeof(un)) < 0){
                perror("bind");
                exit(0);
        }

        un.sun_path[0] = 0;
        strcpy(&un.sun_path[1], "austin");
        len = offsetof(struct sockaddr_un, sun_path) + strlen(&un.sun_path[1]) + 2;
        */

        if(connect(sfd, (void*)&un, len) < 0){
                perror("connect");
                exit(0);
        }

        bzero(&un, sizeof(un));
        len = sizeof(un);
        getpeername(sfd, (void*)&un, &len);
        printf("Len:%d path:%s\n", len, &un.sun_path[1]);

        char buf[CMSG_SPACE(sizeof(int))];
        struct msghdr msg;
        msg.msg_control = buf;
        msg.msg_controllen = sizeof(buf);
        msg.msg_name = NULL;
        msg.msg_namelen = 0;

	struct iovec iov[1];
	iov[0].iov_base = malloc(1024);
	iov[0].iov_len = 1024;
	msg.msg_iov = iov;
        msg.msg_iovlen = 1;

        if((len=recvmsg(sfd, &msg, 0)) < 0){
                perror("recvmsg");
                exit(0);
        }
        printf("recv:%d\n", len);

        if(msg.msg_controllen > 0){
                struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
		if ((cmsg->cmsg_level == SOL_SOCKET) && (cmsg->cmsg_type == SCM_RIGHTS)) {
			int *ptr = (void*)CMSG_DATA(cmsg);
                        char io[1024];
                        while(len=read(*ptr, io, sizeof(io))){
                                if(len < 0){
                                        perror("read");
                                        exit(0);
                                }
                                write(1, io, len);
                        }
		}
	}
        
        return 0;
}