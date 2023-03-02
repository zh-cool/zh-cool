#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>

int str_echo(int peer)
{
	ssize_t len = 0;
	char buf[1024];
	struct timeval tv, t_tv;
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	struct timespec tspec, t_tspec;
	tspec.tv_sec = 5;
	tspec.tv_nsec = 0;

	fd_set rdset, t_rdset;
	FD_ZERO(&rdset);
	FD_SET(peer, &rdset);
	FD_SET(0, &rdset);

	int num = 0;
	for (;;) {
		t_tv = tv, t_rdset = rdset;
		num = select(peer + 1, &t_rdset, NULL, NULL, NULL);
		printf("select %d\n", num);
		if (num > 0) {
			if (FD_ISSET(0, &t_rdset)) {
				len = read(0, buf, sizeof(buf));
				if (write(peer, buf, len) < 0) {
					perror("write peer");
					exit(0);
				}
			}
			if (FD_ISSET(peer, &t_rdset)) {
				len = read(peer, buf, sizeof(buf));
				if (len < 0) {
					perror("cil read");
					exit(0);
				}
                                if(len==0){
                                        perror("Server leave");
                                        exit(0);
                                }
				write(1, buf, len);
			}
		}
	}
}

int main(int argc, char **argv)
{
	struct sockaddr_in srvaddr, localaddr;
	bzero(&srvaddr, 0);

	srvaddr.sin_family = AF_INET;
	inet_pton(AF_INET, argv[1], &srvaddr.sin_addr);
	srvaddr.sin_port = htons(5000);

	localaddr.sin_family = AF_INET;
	localaddr.sin_port = htons(8000);
	inet_pton(AF_INET, "192.168.10.10", &localaddr.sin_addr);

	int fd = socket(AF_INET, SOCK_STREAM, 0);

	if(bind(fd, (void *)&localaddr, sizeof(localaddr)) < 0){
		perror("bind");
	}

	connect(fd, (void *)&srvaddr, sizeof(srvaddr));

	struct linger linger;
	linger.l_linger = 0;
	linger.l_onoff = 1;

	str_echo(fd);

	close(fd);
	return 0;
}