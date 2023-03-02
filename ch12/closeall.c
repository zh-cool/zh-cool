#define _GNU_SOURCE
#include <fcntl.h>
//#include <linux/close_range.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <netdb.h>

#define PRE "SXGD"
#define VERSION PRE"1.2"
/* Show the contents of the symbolic links in /proc/self/fd */
static void show_fds(void)
{
	DIR *dirp = opendir("/proc/self/fd");
	if (dirp == NULL) {
		perror("opendir");
		exit(EXIT_FAILURE);
	}

	for (;;) {
		struct dirent *dp = readdir(dirp);
		if (dp == NULL)
			break;

		if (dp->d_type == DT_LNK) {
			char path[PATH_MAX], target[PATH_MAX];
			snprintf(path, sizeof(path), "/proc/self/fd/%s", dp->d_name);

			ssize_t len = readlink(path, target, sizeof(target));
			printf("%s ==> %.*s\n", path, (int)len, target);
		}
	}

	closedir(dirp);
}

int main(int argc, char *argv[])
{
	for (int j = 1; j < argc; j++) {
		int fd = open(argv[j], O_RDONLY);
		if (fd == -1) {
			perror(argv[j]);
			exit(EXIT_FAILURE);
		}
		printf("%s opened as FD %d\n", argv[j], fd);
	}

	show_fds();

	printf("========= About to call close_range() =======\n");
	printf("GLIVC VERSION:%lu\n", _POSIX_C_SOURCE);
	printf("Versio:%s\n", VERSION);
	/*
	if (syscall(__NR_close_range, 3, ~0U, 0) == -1) {
		perror("close_range");
		exit(EXIT_FAILURE);
	}
	*/
	close_range(0, ~0U, 0);
	getaddrinfo(0, 0, 0, 0 );


	show_fds();
	exit(EXIT_FAILURE);
}
