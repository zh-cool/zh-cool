#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char **argv)
{
        int fd = socket(AF_INET6, SOCK_RAW, IPPROTOCOL_ICMP6);
}
