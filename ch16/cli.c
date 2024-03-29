#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void sig_usr(int sig)
{
	printf("recv sig  %s\n", strsignal(sig));
	return;
}

int str_cli(int conn)
{
	fcntl(conn, F_SETFL, fcntl(conn, F_GETFL, 0) | O_NONBLOCK);
	fcntl(STDIN_FILENO, fcntl(STDIN_FILENO, F_GETFL, 0) | O_NONBLOCK);
	fcntl(STDOUT_FILENO, fcntl(STDOUT_FILENO, F_GETFL, 0) | O_NONBLOCK);

	char to[BUFSIZ] = { 0 };
	char fr[BUFSIZ] = { 0 };
	char *toiptr, *tooptr, *toend;
	char *friptr, *froptr, *frend;
	toiptr = tooptr = to;
	toend = to + BUFSIZ;
	friptr = froptr = fr;
	frend = fr + BUFSIZ;

	int maxfdp1 = conn > STDIN_FILENO ? conn : STDIN_FILENO;
	maxfdp1 = (maxfdp1 > STDOUT_FILENO ? maxfdp1 : STDOUT_FILENO) + 1;
	fd_set rset, wset;
	int len;
	int peeron = 1;

	for(;;){
		FD_ZERO(&rset);
		FD_ZERO(&wset);
		if(toiptr < toend){
			FD_SET(STDIN_FILENO, &rset);
		}
		if(peeron){
			if(friptr < frend){
				FD_SET(conn, &rset);
			}
			if(tooptr < toiptr){
				FD_SET(conn, &wset);
			}
		}
		if(froptr < friptr){
			FD_SET(STDOUT_FILENO, &wset);
		}
		int num = select(maxfdp1, &rset, &wset, NULL, NULL);
		if(num < 0){
			if(errno == EINTR){
				continue;
			}
			perror("select");
			return 0;
		}

		if(FD_ISSET(STDIN_FILENO, &rset)){
			len = read(STDIN_FILENO, toiptr, toend-toiptr);
			if(len < 0){
				perror("read stdin");
			}else if(len == 0){
				printf("Client Terminor");
			}else{
				toiptr += len;
				FD_SET(conn, &wset);
			}
		}
		
		if(FD_ISSET(conn, &rset)){
			len = read(conn, friptr, frend-friptr);
			if(len < 0){
				if(errno != EAGAIN){
					perror("read conn");
				}
			}else if(len == 0){
				peeron  = 0;
				printf("Server Terminor");
			}else{
				friptr += len;
				FD_SET(STDOUT_FILENO, &wset);
			}
		}

		if(FD_ISSET(STDOUT_FILENO, &wset)){
			len = write(STDOUT_FILENO, froptr, friptr-froptr);
			if(len < 0){
				perror("write stdout");
			}else{
				froptr += len;
				if(froptr == friptr){
					froptr = friptr = fr;
				}
			}
		}

		if(FD_ISSET(conn, &wset)){
			len = write(conn, tooptr, toiptr-tooptr);
			if(len < 0){
				perror("write conn");
			}else{
				tooptr += len;
				if(tooptr == toiptr){
					tooptr = toiptr = to;
				}
			}
		}
	}
	
	return 0;
}

int connect_nb(int fd, struct sockaddr *addr, socklen_t addrlen)
{
	int error;
	int flags;
	int len = sizeof(error);
	flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags|O_NONBLOCK);

	if(connect(fd, addr, addrlen) < 0){
		perror("connect_nb_C");
	}

	fd_set wset;
	FD_ZERO(&wset);
	FD_SET(fd, &wset);
	if(select(fd+1, NULL, &wset, NULL, NULL) < 0){
		perror("select");
		return 0;
	}
	if(FD_ISSET(fd, &wset)){
		getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len);
		if(error){
			errno = error;
			perror("connect_nb");
			exit(0);
		}
	}
	fcntl(fd, F_SETFL, flags);
	return fd;
}

int main(int argc, char **argv)
{
	struct addrinfo hints, *result, *rp;

	signal(SIGPIPE, sig_usr);

	bzero(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	int ret = getaddrinfo(argv[1], argv[2], &hints, &result);
	if (ret < 0) {
		perror(gai_strerror(ret));
		exit(0);
	}

	int fd;
	struct timeval tv;
	tv.tv_sec = 500;
	tv.tv_usec = 0;
	for (rp = result; rp; rp = rp->ai_next) {
		fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
			perror("setsockopt");
			exit(0);
		}

		if (connect(fd, rp->ai_addr, rp->ai_addrlen) < 0) {
			perror("connect");
			close(fd);
			continue;
		}
		break;
	}
        if(!rp){
                exit(0);
        }

        struct linger lger;
        lger.l_onoff = 1;
        lger.l_linger = 0;
        setsockopt(fd, SOL_SOCKET, SO_LINGER, &lger, sizeof(lger));
        close(fd);

	//str_cli(fd);
	return 0;
}
