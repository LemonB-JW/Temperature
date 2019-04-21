///*
//This code primarily comes from
//http://www.prasannatech.net/2008/07/socket-programming-tutorial.html
//and
//http://www.binarii.com/files/papers/c_sockets.txt
// */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include "arduino.h"

struct req_info{
  char* request;
  int fd;
};

char* msg;
// extern void* arduino_receive(void*);
// extern int arduino_init();
// extern void arduino_send(void*);
// extern int fd;
// extern arduino_status;

int send_js(char* filename, int fd){
  char* path = malloc(sizeof(char)*100);
  sprintf(path, "../script/%s.js", filename);
  FILE * file = fopen(path, "r");
  if (file == NULL) {
    perror("js file not opened!");
    exit(1);
  }  //get the file size
  fseek(file, 0L, SEEK_END);
  int sz = ftell(file);
  fseek(file, 0L, SEEK_SET);

  char * reply = malloc(sizeof(char) * (sz + 100));
  char * starter = "HTTP/1.1 200 OK\nContent-Type: applicaion/javascript\n\n";
  strcpy(reply, starter);
  char * line = malloc(sizeof(char)*200);
  while (fgets(line, 200, file) != NULL){
    strcat(reply, line);
  }
  send(fd, reply, strlen(reply), 0);
  free(line);
  fclose(file);

  return 0;
}

/* start new thread when new request coming */
void* recv_request(void *new_req) {
  struct req_info * req = (struct req_info *)new_req;
  char* request = malloc(sizeof(char) * (strlen(req->request) + 1));   // TODO: malloc?
  strcpy(request, req->request);
  // print it to standard out
  printf("This is the incoming request:\n%s\n", req->request);

  char* reply;
  // Process request from client
  char mark = request[5];
  printf("token:%c\n", mark);
  //send html back
  if(mark == ' '){
    printf("Reques:Refresh!\n");
    FILE * html = fopen("../script/browser.html", "r");
    if (html == NULL) {
      printf("html file not open!");
      exit(1);
    } else {
      printf("html file opened successfully!");
    }
    //get the file size
    fseek(html, 0L, SEEK_END);
    int sz = ftell(html);
    fseek(html, 0L, SEEK_SET);

    char * reply = malloc(sizeof(char) * (sz + 100));
    char * starter = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n";
    strcpy(reply, starter);
    char * line = malloc(sizeof(char)*200);
    while (fgets(line, 200, html) != NULL){
      strcat(reply, line);
    }
    printf("Reply: %s\n", reply);
    printf("fd: %d\n", req->fd);
    int status = send(req->fd, reply, strlen(reply), 0);
    if (status < 0) {
      printf("send response failed: %s\n", strerror(errno));
    }
    free(line);
    fclose(html);
  } else if (mark == 'b') {
    char* token = strtok(request, " ");
    token = strtok(NULL, " ");
    printf("%s", token);
    if (strcmp("/browser.js", token) == 0) {
      send_js("browser", req->fd);

    }
  } else if (mark == 'T') {
    // Check if arduino is connected
    if (arduino_status == 1) {
      char* error = "Arduino is currently disconnected!";  //TODO: html or something else
      reply = malloc(sizeof(char) * (strlen(error) + 1));
      strcpy(reply, error);
      send(fd, reply, strlen(reply), 0);
      exit(1);
    }
    printf("Dispay: ");
    char reply_head[200] = "HTTP/1.1 200 OK\nContent-Type: apllication/json\n\n";
    char *reply_tail = "{\"curr\":30}";
    // strcat(reply_head, msg);
    strcat(reply_head, reply_tail);
    reply = malloc(sizeof(char)*strlen(reply_head) + 1);
    strcpy(reply, reply_head);
    printf("%s\n", reply);
    // 6. send: send the outgoing message (response) over the socket
// note that the second argument is a char*, and the third is the number of chars
    send(req->fd, reply, strlen(reply), 0);
    free(reply);
  } else if(mark == 'F') {
      printf("ToFah\n");
      arduino_send("F");
  } else if (mark == 'S') {   // change stand by mode
      printf("Stand_by mode change\n");
      arduino_send("S");
  } else if (mark == 'X'){
      printf("CIS\n");     // Tell arduino to display "CIS"
      arduino_send("X");
  } else {
      char high_temp[5];
      char low_temp[5];
      high_temp[0] = request[5];
      high_temp[1] = request[6];
      high_temp[2] = '\0';
      char* high_signal = malloc(sizeof(char)*6);
      strcat(high_signal, "H:");
      strcat(high_signal, high_temp);
      low_temp[0] = request[8];
      low_temp[1] = request[9];
      low_temp[2] = '\0';
      char* low_signal = malloc(sizeof(char)*6);
      strcat(low_signal, "L:");
      strcat(low_signal, low_temp);
      printf("%s %s\n", high_signal, low_signal);

      arduino_send(high_signal);
      arduino_send(low_signal);
      free(high_signal);
      free(low_signal);
  }
  free(request);
  return NULL;
//        printf("%s\n", reply);
}


int start_server(int PORT_NUMBER)
{

      // structs to represent the server and client
      struct sockaddr_in server_addr,client_addr;

      int sock; // socket descriptor

      // 1. socket: creates a socket descriptor that you later use to make other system calls
      if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
          perror("Socket");
          exit(1);
      }
      int temp;
      if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&temp,sizeof(int)) == -1) {
          perror("Setsockopt");
          exit(1);
      }

      // configure the server
      server_addr.sin_port = htons(PORT_NUMBER); // specify port number
      server_addr.sin_family = AF_INET;
      server_addr.sin_addr.s_addr = INADDR_ANY;
      bzero(&(server_addr.sin_zero),8);

      // 2. bind: use the socket and associate it with the port number
      if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
          perror("Unable to bind");
          exit(1);
      }

      // 3. listen: indicates that we want to listen to the port to which we bound; second arg is number of allowed connections
      if (listen(sock, 1) == -1) {
          perror("Listen");
          exit(1);
      }

      // once you get here, the server is set up and about to start listening
      printf("\nServer configured to listen on port %d\n", PORT_NUMBER);
      fflush(stdout);


      arduino_init();
      pthread_t arduino;
      pthread_create(&arduino, NULL, &arduino_receive, NULL);

      int sin_size = sizeof(struct sockaddr_in);

      pthread_t threads[25];
      int i = 0;

      while (1) {
        // 4. accept: wait here until we get a connection on that port
        // Select
          int fd = accept(sock, (struct sockaddr *)&client_addr,(socklen_t *)&sin_size);
          if (fd != -1) {
              printf("Server got a connection from (%s, %d)\n", inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
          struct req_info req;
          req.fd = fd;
          printf("fd: %d\n", req.fd);
          // buffer to read data into
          char request[1024];

          // 5. recv: read incoming message (request) into buffer
          int bytes_received = recv(fd,request,1024,0);
          // null-terminate the string
          request[bytes_received] = '\0';
          req.request = malloc(sizeof(char)*(strlen(request)+1));
          strcpy(req.request, request);

          if (i >= 25) {
            for (int i = 0; i < 25; i++) {
              pthread_join(threads[i], NULL);
            }
          }

          pthread_create(&threads[i], NULL, &recv_request, &req);
          pthread_join(threads[i++], NULL);
          // 7. close: close the connectionu
          close(fd);
          printf("Server closed connection\n");
        }
    }
    pthread_join(arduino, NULL);
      // 8. close: close the socket
    close(sock);
    printf("Server shutting down\n");

    return 0;
}



int main(int argc, char *argv[])
{
  // check the number of arguments
  if (argc != 2) {
      printf("\nUsage: %s [port_number]\n", argv[0]);
      exit(-1);
  }

  int port_number = atoi(argv[1]);
  if (port_number <= 1024) {
    printf("\nPlease specify a port number greater than 1024\n");
    exit(-1);
  }

  start_server(port_number);
}
