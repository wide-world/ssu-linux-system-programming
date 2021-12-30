#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <utime.h>
#include <dirent.h>

int main(int argc, char *argv[])
{
	struct utimbuf time_buf;
	DIR *dp;
	int fd;

	if (argc < 2) {
		fprintf(stderr, "usage: <%s> <file1 or dir1> ... <fileN or dirN>\n", argv[0]);
		exit(1);
	}

	for (int i = 1; i < argc; i++) {
		if ((fd = open(argv[i], O_RDWR|O_CREAT, 0664)) < 0) {
			if ((dp = opendir(argv[i])) == NULL) {
				fprintf(stderr, "open error for %s\n", argv[i]);
				exit(1);
			}
			closedir(dp);
		}
		close(fd);

		if (utime(argv[i], NULL) < 0) {
			fprintf(stderr, "utime error for %s\n", argv[i]);
			exit(1);
		}
	}

	return 0;
}