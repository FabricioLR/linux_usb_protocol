#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define DEFAULT_DEVICE "/dev/ml0"

int main(){
	int fd, cmd;
	char *dev = DEFAULT_DEVICE;
	unsigned char buffer[1024];

	printf("Open device %s\n", dev);
	fd = open(dev, O_RDWR);
	if (fd == -1) {
		perror("open");
		exit(1);
	}

	int length = read(fd, buffer, 1024);

	for (int i = 0; i < length; i++){
		printf("%c", buffer[i]);
	}

	printf("\n");

	close(fd);

	return EXIT_SUCCESS;
}