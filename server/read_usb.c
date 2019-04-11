#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

int fd;
char* msg;
pthread_mutex_t fd_lock;
/*
This code configures the file descriptor for use as a serial port.
*/
void configure(int fd) {
  struct termios pts;
  tcgetattr(fd, &pts);
  cfsetospeed(&pts, 9600);
  cfsetispeed(&pts, 9600);

  /*
  // You may need to un-comment these lines, depending on your platform.
  pts.c_cflag &= ~PARENB;
  pts.c_cflag &= ~CSTOPB;
  pts.c_cflag &= ~CSIZE;
  pts.c_cflag |= CS8;
  pts.c_cflag &= ~CRTSCTS;
  pts.c_cflag |= CLOCAL | CREAD;
  pts.c_iflag |= IGNPAR | IGNCR;
  pts.c_iflag &= ~(IXON | IXOFF | IXANY);
  pts.c_lflag |= ICANON;
  pts.c_oflag &= ~OPOST;
  */

  tcsetattr(fd, TCSANOW, &pts);

}

int arduino_init() {
    pthread_mutex_init(&fd_lock, NULL);
    // get the name from the command line

    char* filename = "/dev/cu.usbmodem1421";

    // try to open the file for reading and writing
    // you may need to change the flags depending on your platform
    pthread_mutex_lock(&fd_lock);
    fd = open(filename, O_RDWR | O_NOCTTY | O_NDELAY);
    pthread_mutex_unlock(&fd_lock);


    if (fd < 0) {
        perror("Could not open file\n");
        exit(1);
    }
    else {
        printf("Successfully opened %s for reading and writing\n", filename);
    }

    configure(fd);



    return 0;
}

void arduino_send(char* signal) {
    pthread_mutex_lock(&fd_lock);
    int byte_written = write(fd, signal, strlen(signal));
    pthread_mutex_unlock(&fd_lock);

    printf("signal %s sent\n", signal);
    printf("byte_written: %d\n", byte_written);

}

void* arduino_receive(void* arg) {
    char buf;

    int bytes_read = 0;
    char* tmp = malloc(100*sizeof(char));
    msg = malloc(100*sizeof(char));
    // "\n\n"
    int hit_n = 0;

    while(1) {
        int i = 0;
        memset(tmp, '\0', 100*sizeof(char));
        while (1) {
            pthread_mutex_lock(&fd_lock);
            bytes_read = read(fd, &buf, 1);
            pthread_mutex_unlock(&fd_lock);
            if (bytes_read <= 0) {
                continue;
            }
            if (buf == '\n') {
                if (hit_n == 0) {
                    hit_n = 1;
                    continue;
                }
                if (hit_n == 1) {
                    printf("hit new line %s\n", msg);
                    hit_n = 0;
                    break;
                }
            }
            tmp[i++] = buf;
        }
        strcpy(msg, tmp);
    }
    return NULL;
}
