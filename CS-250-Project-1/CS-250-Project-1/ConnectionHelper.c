//
//  ConnectionHelper.c
//  CS-250-Project-1
//
//  Created by william wright on 11/15/18.
//  Copyright © 2018 Silvertap. All rights reserved.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define PORT     5000
#define MAXLINE 1024
int recieveint(int sockfd,struct sockaddr_in servaddr);
void sendandrecieve(int sockfd,char input[MAXLINE],char output[MAXLINE],struct sockaddr_in servaddr);
char* gettime(int type){
    int sockfd;
    char buffer[MAXLINE];
    char *hello = "Hello from client";
    if(type==1){
        hello = "GMT";
    }else if(type==2){
        hello = "Local";
    }
    struct sockaddr_in     servaddr;
    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    memset(&servaddr, 0, sizeof(servaddr));
    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5000);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    int n, len;
    sendto(sockfd, (const char *)hello, strlen(hello),0, (const struct sockaddr *) &servaddr,sizeof(servaddr));
    n = recvfrom(sockfd, (char *)buffer, MAXLINE,0, (struct sockaddr *) &servaddr,&len);
    buffer[n] = '\0';
    close(sockfd);//close the udp socket
    printf("\n%s\n",buffer);//print the time recieved from udp
    return buffer;
}
//in this method we use tcp to connect to the server and request quote of the day
//we receave the string encripted so we have to send it to the decripter to process the string
void getqotd(){
    
    int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr;
    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket creation failed");
        return;
    }
    memset(&servaddr, 0, sizeof(servaddr));
    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (connect(sockfd, (struct sockaddr*)&servaddr,
                sizeof(servaddr)) < 0) {//lets try and connect to the server
        printf("\n Error : Connect Failed \n");
    }
    memset(buffer, 0, sizeof(buffer));
    strcpy(buffer, "QOTD");
     CaesarCipher(1, buffer);
    send(sockfd, buffer, sizeof(buffer),0);//send the hello to the server
    memset(buffer, 0, MAXLINE);
    read(sockfd, buffer, sizeof(buffer));//receive the tcp encripted message from the server
    CaesarCipher(2, buffer);//decript the message
    close(sockfd);
    char counter[2];
    counter[0]=buffer[0];
    counter[1]=buffer[1];
    int numchoice;
    char QOTD[strlen(buffer-2)];
    for(int i=2;i<strlen(buffer)-2;i++){
        QOTD[i-2]=buffer[i];
    }
    printf("|-------------------------------------------------------------------------|\n");
    printf("|   Quote Of The Day:%s\n",QOTD);
    printf("|-------------------------------------------------------------------------|\n");
    
    do{
        printf("|-----------------------------------------------------|\n");
        printf("|                Quote Of The Day Menu                |\n");
        printf("|     Please Pick A Number From The Options Below     |\n");
        printf("|                                                     |\n");
        printf("|     1.) Select To Like The Quote Of The Day         |\n");
        printf("|     2.) Select To Dislike The Quote Of The Day      |\n");
        printf("|     3.) Select To Just Return To The Main Menu      |\n");
        printf("|     4.) List All Quotes Disliked                    |\n");
        printf("|-----------------------------------------------------|\n");
        printf("Option: ");
        scanf("%d", &numchoice);
        if(numchoice==1||numchoice==2||numchoice==3||numchoice==4){
            if(numchoice==1){
                char message[10];
                sprintf(message, "LIKE%s",counter);
               char output5[MAXLINE];
                sendandrecieve(sockfd, message, output5, servaddr);
                printf("%s\n",output5);
                numchoice=3;
            }else if(numchoice==2){
                char message[MAXLINE];
                sprintf(message, "DISLIKE%s",counter);
                char output[MAXLINE];
                sendandrecieve(sockfd, message, output, servaddr);
                printf("%s",output);
                numchoice=3;
            }else if(numchoice==4){
                char message[]="LISTALL";
                char output[MAXLINE];
                sendandrecieve(sockfd, message, output, servaddr);
                printf("%s",output);
                numchoice=3;
            }else{
                scanf("Please Enter One Of The Options");
            }
        }
    }while(numchoice !=3);
     char output[MAXLINE];
    char message[] = "EXIT";
    //sendandrecieve(sockfd, message, output, servaddr);
    return;
}






void sendandrecieve(int sockfd,char input[],char output[],struct sockaddr_in servaddr){
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket creation failed");
        return;
    }
    if (connect(sockfd, (struct sockaddr*)&servaddr,
                sizeof(servaddr)) < 0) {//lets try and connect to the server
        printf("\n Error : Connect Failed \n");
        return;
    }
    CaesarCipher(1,input);
    write(sockfd, input, strlen(input));//send the tcp encripted message to the server
    read(sockfd, output, MAXLINE);//receive the tcp encripted message from the server
    CaesarCipher(2,output);
    close(sockfd);
    return;
}










int recieveint(int sockfd,struct sockaddr_in servaddr){
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket creation failed");
        return 0;
    }
    if (connect(sockfd, (struct sockaddr*)&servaddr,
                sizeof(servaddr)) < 0) {//lets try and connect to the server
        printf("\n Error : Connect Failed \n");
    }
    char mess[]="LLS";
   // CaesarCipher(2,mess);
    write(sockfd, mess, MAXLINE);//send the tcp encripted message to the server
    int n;
    int received_int = 0;
    n = read(sockfd, &received_int, sizeof(received_int));
    if (n > 0) {
        close(sockfd);
       return ntohl(received_int);
    }
    else {
        // Handling erros here
    }
    close(sockfd);
    return 0;
}
