#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <netinet/ether.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/select.h>

#define DAT_LEN 56
char buf[BUFSIZ] = { 0 };

uint16_t checksum(uint16_t *addr, int len)
{
	int count = len;
	register uint32_t sum = 0;
	uint16_t answer = 0;

	// Sum up 2-byte values until none or only one byte left.
	while (count > 1) {
		sum += *(addr++);
		count -= 2;
	}

	// Add left-over byte, if any.
	if (count > 0) {
		sum += *(uint8_t *)addr;
	}

	// Fold 32-bit sum into 16 bits; we lose information by doing this,
	// increasing the chances of a collision.
	// sum = (lower 16 bits) + (upper 16 bits shifted right 16 bits)
	while (sum >> 16) {
		sum = (sum & 0xffff) + (sum >> 16);
	}

	// Checksum is one's compliment of sum.
	answer = ~sum;

	return (answer);
}

int send_v4(int fd, uint16_t seq, struct addrinfo *peer)
{
	uint16_t sum;
	struct icmp *icmp = (void *)buf;
	icmp->icmp_type = ICMP_ECHO;
	icmp->icmp_code = 0;
	icmp->icmp_id = htons(getpid());
	icmp->icmp_seq = htons(seq);
	icmp->icmp_cksum = 0;
	memset(icmp->icmp_data, 0x5A, DAT_LEN);
	gettimeofday((void *)icmp->icmp_data, NULL);
	sum = checksum((void *)icmp, 8 + DAT_LEN);
	icmp->icmp_cksum = sum;
	return sendto(fd, icmp, 8 + DAT_LEN, 0, peer->ai_addr, peer->ai_addrlen);
}

int proc_v4(struct msghdr *msg, int rlen)
{
	struct ip *ip = msg->msg_iov->iov_base;
	int pktsize = rlen - (ip->ip_hl<<2);
	char peer[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &ip->ip_src, peer, sizeof(peer));
	struct icmp *icmp =(void*) ((char*)ip + (ip->ip_hl<<2));
	struct timeval t1, *t2;
	gettimeofday(&t1, NULL);
	t2 = (void*)&icmp->icmp_data;
	double msec = (t1.tv_sec - t2->tv_sec)*1000 + (t1.tv_usec - t2->tv_usec)/1000.0;
	printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.2f\n", rlen, peer, ntohs(icmp->icmp_seq), ip->ip_ttl, msec);
	return 0;
}

int send_v6(int fd, uint16_t seq, struct addrinfo *peer)
{
	struct icmp6_hdr *icmp = (void*)buf;
	icmp->icmp6_type= ICMP6_ECHO_REQUEST;
	icmp->icmp6_code= 0;
	icmp->icmp6_cksum = 0;
	icmp->icmp6_id = htons(getpid());
	icmp->icmp6_seq = htons(seq);
	memset(icmp+1, 0x58, DAT_LEN);
	gettimeofday((void*)(icmp+1), NULL);
	return sendto(fd, icmp, 8+DAT_LEN, 0, peer->ai_addr, peer->ai_addrlen);
}

int proc_v6(struct msghdr *msg, int rlen)
{
	char ip[128];
	struct icmp6_hdr *icmp = msg->msg_iov->iov_base;

	if(icmp->icmp6_id != htons(getpid())){
		return 0;
	}
	struct cmsghdr *cmsg;
	int32_t ttl;
	for(cmsg=CMSG_FIRSTHDR(msg); cmsg; cmsg=CMSG_NXTHDR(msg, cmsg)){
		if(cmsg->cmsg_level== IPPROTO_IPV6 && cmsg->cmsg_type==IPV6_HOPLIMIT){
			ttl = *(int32_t*)CMSG_DATA(cmsg);
		}
		break;
	}
	getnameinfo(msg->msg_name, msg->msg_namelen, ip, sizeof(ip), NULL, 0, NI_NUMERICHOST);
	struct timeval t1, *t2;
	gettimeofday(&t1, NULL);
	t2 = (void*)(icmp+1);
	double msec = (t1.tv_sec - t2->tv_sec)*1000 + (t1.tv_usec - t2->tv_usec)/1000.0;
	int anclen = msg->msg_controllen;
	printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.2f anc:%d\n", rlen, ip, ntohs(icmp->icmp6_seq), ttl, msec, anclen);
	return 0;
}

int (*proc_icmp)(struct msghdr *msg, int rlen);
int (*send_icmp)(int fd, uint16_t seq, struct addrinfo *peer);

void run_loop(int fd, struct addrinfo *peer)
{
	struct timeval tv;
	fd_set rset, trset;
	FD_ZERO(&rset);
	FD_SET(fd, &rset);
	tv.tv_sec = 1;
	tv.tv_usec = 0;

	uint16_t seq = 1;
	int num = 0;
	for (;;) {
		trset = rset;
		num = select(fd + 1, &trset, 0, 0, &tv);
		if (num < 0) {
			perror("select");
			exit(0);
		}
		if (num == 0) {
			send_icmp(fd, seq++, peer);
			tv.tv_sec = 1;
			tv.tv_usec = 0;
			continue;
		}

		struct sockaddr_storage cli;
		int rlen = 0;
		char anc[128];
		struct iovec iov = {buf, sizeof(buf)};
		struct msghdr msg;
		msg.msg_name = &cli;
		msg.msg_namelen = sizeof(cli);
		msg.msg_control = anc;
		msg.msg_controllen = sizeof(anc);
		msg.msg_iov = &iov;
		msg.msg_iovlen = 1;
		if((rlen = recvmsg(fd, &msg, 0)) > 0){
			printf("flags:%02x\n", msg.msg_flags);
			proc_icmp(&msg, rlen);
		}	
	}
}

int main(int argc, char **argv)
{
	char buf[BUFSIZ] = { 0 };
	int ret, fd;

	struct addrinfo hints, *result, *rp;
	bzero(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_RAW;
	//hints.ai_protocol = IPPROTO_ICMP;
	char *host = NULL;
	if (argc <= 3) {
		host = argv[1];
	}

	if ((ret = getaddrinfo(host, NULL, &hints, &result)) < 0) {
		perror(gai_strerror(ret));
		exit(0);
	}

	for (rp = result; rp; rp = rp->ai_next) {
		if ((fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_family == AF_INET ? IPPROTO_ICMP : IPPROTO_ICMPV6)) < 0) {
			perror("socket");
			continue;
		}
		if(rp->ai_family == AF_INET){
			send_icmp = send_v4;
			proc_icmp = proc_v4;
		} else {
			int on = 1;
			setsockopt(fd, IPPROTO_IPV6, IPV6_RECVHOPLIMIT, &on, sizeof(on));
			send_icmp = send_v6;
			proc_icmp = proc_v6;
		}
		break;
	}
	if (!rp) {
		return 0;
	}

        run_loop(fd, rp);

	return 0;
}
