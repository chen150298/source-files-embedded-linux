#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "communication.h"


int writeToLog(FILE *fp,char * buffer);
void readLog(FILE * logfile, int lines_req, Messages* messages);
int checkRead (char* buffer, int* num);
int checkWrite (char* buffer, char* log);

int main() {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    char buffer[MAXLINE];
    int len, n;
    FILE* logfile;
    
    // Creating socket file descriptor
    // AF_INET -> IPv4 x.x.x.x  SOCK_DGRAM -> UDP
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        goto socket_err;
    }

    // initialize memory to zero for the server address structure and client address structure 
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);    // listens to server ip
    servaddr.sin_port = htons(SERVER_PORT);    // htons converts the port number from host byte order to network byte order

    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        goto socket_err;
    }

    // Open the log file in append mode or create if it doesn't exist
    logfile = fopen("log", "a+");
    if (logfile == NULL) {
        perror("Error opening log file");
        goto openf_error;
    }

    printf("Server listening on %s:%d\n", SERVER_IP, SERVER_PORT);

    len = sizeof(cliaddr);
    int num;
    Messages messages;
    char log[MAXLINE];
    while (1) {      
        n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, (struct sockaddr *)&cliaddr, &len);
        if (n < 0) exit(1); // TODO: release all objects, to change
        buffer[n] = '\0';

        char* clieIP = inet_ntoa(cliaddr.sin_addr);
        printf("Received: %s from: %s\n", buffer, clieIP);

        if (strcmp(clieIP, CLIENT_IP) == 0) {
             if (strncmp(buffer,"read",4) == 0) {
                // parse + sent last log + ack
                if (checkRead(buffer, &num)) goto while_err;
                readLog(logfile, num, &messages);
                sendto(sockfd, &messages, sizeof(struct Messages), 0, (const struct sockaddr *)&cliaddr, len);
                sendto(sockfd, "ack", strlen("ack"), 0, (const struct sockaddr *)&cliaddr, len);
            } 
            if (strncmp(buffer,"write",5) == 0){
                // parse + write to log + ack
                if (checkWrite (buffer, log)) goto while_err;
                if(writeToLog(logfile, log)) goto while_err;
                sendto(sockfd, "ack", strlen("ack"), 0, (const struct sockaddr *)&cliaddr, len);
            }             
        } 
    }

    close(sockfd);
    return 0;

while_err:
    fclose(logfile);  
openf_error:
    close(sockfd);
socket_err:
    exit(EXIT_FAILURE);

}

int checkRead (char* buffer, int* num) {
    int status = sscanf(buffer, "read %d", num);
    if (status < 1) {
        printf("sscanf read failed\n");
        return 1;
    }
    if (*num <= 0 || *num > 8) {
        return 1;
        printf("number not between 1 to 8\n");
    }
    return 0;
}

int checkWrite (char* buffer, char* log) {
    int status = sscanf(buffer, "write %s", log);
    if (status < 1) {
        printf("sscanf write failed\n");
        return 1;
    }
    return 0;
}

int writeToLog(FILE* logfile,char * buffer){
    int bytes_written = fprintf(logfile, "%s\n", buffer);
    if (bytes_written < 0) {
        perror("fprintf failed");
        return 1;
    }
    fflush(logfile);
    return 0;
}

void readLog(FILE * logfile, int lines_req, Messages* messages) {
    fseek(logfile, 0, SEEK_END);
    long pos = ftell(logfile);
    int lines_number = 0;
    char c;
    char buffer[MAXLINE];
    int index = MAXLINE - 1;

    fseek(logfile, --pos, SEEK_SET);
    while (pos && lines_number < lines_req) {
        fseek(logfile, --pos, SEEK_SET);    // Move the pointer position one character back
        c = fgetc(logfile);    // Read one character
        index--;
        buffer[index] = c;  // fill buffer
        if ((c == '\n' && index++) || pos == 0) {    // reached line before or start of file
            buffer[MAXLINE - 1] = '\0';
            // enter line to struct to send 
            memcpy(&(messages->messsages_array[lines_number]), &buffer[index], MAXLINE - index + 1);
            index = MAXLINE - 1;
            lines_number++; 
        }
    }
    messages->lines_num = lines_number;
    // Move the pointer to the end of the file
    fseek(logfile, pos, SEEK_END);
}