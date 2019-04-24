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
#include <time.h>
#include "temp_calc.h"

extern int isF;
int fd;
extern char* msg;
double cur_temp;
pthread_mutex_t fd_lock;
pthread_mutex_t cur_temp_lock;
pthread_mutex_t arduino_status_lock;
extern pthread_mutex_t isF_lock;

int arduino_status;  // 0 if Arduino connected, 1 if disconnected
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


double get_cur_temp(char* s) {
    char* tmp = NULL;
    double temp = 0;
    if (s != NULL && strlen(s) > 15) {  // Temp:26 degrees C
      tmp = strtok(s, ":");
      tmp = strtok(NULL, " ");
    }
    if (tmp != NULL) {
     temp = atof(tmp);
    }
    if (temp != 0) {
      int status = update_temp(&temp);
      if (status != 0) {
        perror("update temperature failed");
        return 1;
      }
      printf("get_temp: %f\n", temp);
      return temp;
    }
    return 1;  // if not return temp
}


int arduino_init() {
    pthread_mutex_init(&fd_lock, NULL);
    pthread_mutex_init(&cur_temp_lock, NULL);
    pthread_mutex_init(&arduino_status_lock, NULL);
    // get the name from the command line

    char* filename = "/dev/cu.usbmodem1421";

    // try to open the file for reading and writing
    // you may need to change the flags depending on your platform
    pthread_mutex_lock(&fd_lock);
    fd = open(filename, O_RDWR | O_NOCTTY | O_NDELAY);
    pthread_mutex_unlock(&fd_lock);


    if (fd < 0) {
        perror("Could not open file\n");
        pthread_mutex_lock(&arduino_status_lock);
        arduino_status = 1;   // disconnected
        pthread_mutex_unlock(&arduino_status_lock);
        return 1;
    }
    else {
        printf("Successfully opened %s for reading and writing\n", filename);
        arduino_status = 0;
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
    clock_t last_time;

    while(1) {
        int i = 0;
        memset(tmp, '\0', 100*sizeof(char));
        while (1) {
            pthread_mutex_lock(&fd_lock);
            bytes_read = read(fd, &buf, 1);
            pthread_mutex_unlock(&fd_lock);

            if (bytes_read <= 0) {
                // Check if Arduino is Connected
                if (((double)(clock() - last_time) / CLOCKS_PER_SEC) > 3) {
                    while (arduino_init() != 0) {
                        printf("hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh\n");
                        pthread_mutex_lock(&arduino_status_lock);
                        arduino_status = 1;    // Mark disconnected
                        pthread_mutex_unlock(&arduino_status_lock);

                        pthread_mutex_lock(&isF_lock);
                        isF = 0;
                        pthread_mutex_unlock(&isF_lock);
                        sleep(1);
                        // continue;
                    }
                } else {
                    pthread_mutex_lock(&arduino_status_lock);
                    arduino_status = 0;
                    pthread_mutex_unlock(&arduino_status_lock);

                    continue;
                }
            }
            pthread_mutex_lock(&arduino_status_lock);
            arduino_status = 0;
            pthread_mutex_unlock(&arduino_status_lock);

            // If we receive message from Arduino successfully, we record the current time
            last_time = clock();
            if (buf == '\n') {
                if (hit_n == 0) {
                    hit_n = 1;
                    continue;
                }
                if (hit_n == 1) {
                    hit_n = 0;
                    break;
                }
            }
            tmp[i++] = buf;
        }
        tmp[i] = '\0';
        strcpy(msg, tmp);
        printf("%s\n", msg);

        pthread_mutex_lock(&cur_temp_lock);
        cur_temp = get_cur_temp(msg);  // extract current temperature from the input
        pthread_mutex_unlock(&cur_temp_lock);

        printf("isF %d\n", isF);
    }
    return NULL;
}
