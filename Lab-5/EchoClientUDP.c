#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

#define ECHOMAX 255     /* Longest string to echo */

void DieWithError(char *errorMessage);  /* External error handling function */

int main(int argc, char *argv[])
{
    unsigned short echoServPort;        /* Server port */
    /* ------Step 0 check user input ------ */
    /* Test for correct number of arguments from user */
    if (argc != 2) {//check to see if only the port arguemnt was entered
        fprintf(stderr, "Usage:  %s <UDP Server Port>\n", argv[0]);//get the port number from the srguments passed into main
        exit(1);//exit the program
    }
    echoServPort = atoi(argv[1]);      /* First arg:  should be local port */
    int sockfd; //define the socket
    char buffer[ECHOMAX];//create the buffer
    char *hello = "Hello from client3";
    struct sockaddr_in     servaddr; //define the socket
    /* ------Step 1 create the socket ------- */
    /* Create socket for outgoing connections */
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);//exit the program due to socket failure
    }
    
    memset(&servaddr, 0, sizeof(servaddr));
    // Filling server information
    /* Construct local address structure */
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(echoServPort);
    //change if you want to test ovwer a network
    servaddr.sin_addr.s_addr = inet_addr("192.168.0.24");//deifne thew port of sending to localhost for testing
    
    int n, len;
    sendto(sockfd, (const char *)hello, strlen(hello),0, (const struct sockaddr *) &servaddr,sizeof(servaddr));
    n = recvfrom(sockfd, (char *)buffer, ECHOMAX,0, (struct sockaddr *) &servaddr,&len);
    buffer[n] = '\0';
    close(sockfd);//close the udp socket
    printf("|------------------------------------------------------------------------------------------|\n");
    printf("|                                CS-250 Project-1 UDP-Client                               |\n");
    printf("|                                Developed By William Wright                               |\n");
    printf("|                  When Launched The client will send a udp browdcast on 5000              |\n");
    printf("|------------------------------------------------------------------------------------------|\n");
    printf("From Client: %s\n",hello);//print the time recieved from udp
    printf("From Server: %s\n",buffer);//print the time recieved from udp
    /* ------Step 5 implied close - will require cntrl-c or system interrupt to exit  ------- */
    exit(1);
}
