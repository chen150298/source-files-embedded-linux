#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "communication.h"

int checkUserRead (char* buffer);
int parseServerRead (char* buffer);
int waitForAck(int sockfd);
int checkUserWrite (char* buffer);
void printMsgs(Messages msgs);

int main() {
    int sockfd;
    struct sockaddr_in servaddr;
    char buffer[MAXLINE];
    int len, n;

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Filling server information - to send data
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    servaddr.sin_port = htons(SERVER_PORT);

    printf("Client sending messages to %s:%d\n", SERVER_IP, SERVER_PORT);

    Messages msgs;
    while (1) {
            printf("Enter message to send: ");
            fgets(buffer, MAXLINE, stdin);
            printf("here1!\n");

            
            if (strncmp(buffer,"read",4) == 0) {
                if (checkUserRead(buffer)) exit(1); // TODO: release all objects
                sendto(sockfd, (const char *)buffer, strlen(buffer), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr)); // Send message to server
                n = recvfrom(sockfd, (Messages*) &msgs, sizeof(struct Messages), 0, NULL, NULL);
                if (n <= 0) exit(1); // TODO: release all objects
                if (waitForAck(sockfd)) exit(1); // TODO: release all objects
                printMsgs(msgs);
            }

            
            if (strncmp(buffer,"write",5) == 0) {
                printf("i am writing to server!");
                if (checkUserWrite (buffer)) exit(1); // TODO: release all objects
                sendto(sockfd, (const char *)buffer, strlen(buffer), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
                if (waitForAck(sockfd)) exit(1); // TODO: release all objects
            }

            
            if (strncmp(buffer,"exit",4) == 0) {    // TODO: delete!!
                sendto(sockfd, (const char *)buffer, strlen(buffer), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
            }
            
    }

    close(sockfd);
    return 0;
}

int checkUserRead (char* buffer) {
    int num;
    int status = sscanf(buffer, "read %d", &num);
    if (status < 1) {
        return 1;
    }
    if (num <= 0 || num > 8) {
        return 1;
    }
    return 0;
}

 int waitForAck(int sockfd) {
    char buffer[MAXLINE];
    int n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, NULL, NULL);
    if (n < 0 || strncmp(buffer, "ack", 3) != 0){
        return 1;
    }
    return 0;
 }

 int checkUserWrite (char* buffer) {
    char log[MAXLINE];
    int status = sscanf(buffer, "write %s", log);
    if (status < 1) {
        return 1;
    }
    return 0;
}

void printMsgs(Messages msgs) {
    int num = msgs.lines_num;
    printf("num of lines in client: %d\n", num);
    for (int i = 0; i < num; i++) {
        printf("log %d: %s\n", i, msgs.messsages_array[i]);
    }
}
