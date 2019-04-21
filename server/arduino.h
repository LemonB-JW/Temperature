int fd;
char* msg;
pthread_mutex_t fd_lock;
int arduino_status;  // 0 if Arduino connected, 1 if disconnected


void configure(int fd);
int arduino_init();
void arduino_send(char* signal);
void arduino_send(char* signal);
void* arduino_receive(void* arg);
