#define SERVER_IP "10.0.2.2" // Server IP address
#define CLIENT_IP "10.0.2.1"  // my client ip - mykernel
#define SERVER_PORT 8080
#define MAXLINE 64

typedef struct Messages{
    char messsages_array[8][MAXLINE];
    int lines_num;
} Messages;
