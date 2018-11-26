//
//  Client.c
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
#define MAXLINE 1024

//---------------------------------------------See Methods Below For More Information-----------------------------------------------------------------------------------------------------|
void sendandrecieve(int sockfd,char input[MAXLINE],char output[MAXLINE],struct sockaddr_in servaddr);//the main function for seinding data and geting data from the sevrer |
//---------------------------------------------See Methods Below For More Information-----------------------------------------------------------------------------------------------------|

//-----------------------------------------------------------------------------------------------|
//                                           gettime();                                          |
//     This Function Sends a UDP Connection to the server and receives the time Requested        |
//-----------------------------------------------------------------------------------------------|
void gettime(int type,int PORT,char address[]){
    //this function is called from main with the type of time to respomd with
    int sockfd;//define the socket
    char buffer[MAXLINE];//define buffer
    char *hello = "Defualt";//our deafult value
    if(type==1){
        hello = "GMT";//set the message to GMT
    }else if(type==2){
        hello = "Local";//set the message to LOCAL time
    }
    struct sockaddr_in servaddr;
    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    memset(&servaddr, 0, sizeof(servaddr));
    // Declare The server information
    servaddr.sin_family = AF_INET;//the type of connection we will be using
    servaddr.sin_port = htons(PORT);//set the port of the server
    servaddr.sin_addr.s_addr = inet_addr(address);//set the adress the server is on
    int n, len;
    //send the Request to the Server
    sendto(sockfd, (const char *)hello, strlen(hello),0, (const struct sockaddr *) &servaddr,sizeof(servaddr));
    n = recvfrom(sockfd, (char *)buffer, MAXLINE,0, (struct sockaddr *) &servaddr,&len);//receive the Time From the server
    buffer[n] = '\0';
    close(sockfd);//close the udp socket
    printf("\n%s\n",buffer);//print the time recieved from udp
    return;
}
//-----------------------------------------------------------------------------------------------|
//                                           getQOTD();                                          |
//     This Function Sends a TCP Connection to the server and receives the Quote of the day      |
//-----------------------------------------------------------------------------------------------|
void getqotd(int PORT,char address[]){
    int sockfd;//the srtucture deffanation
    char buffer[MAXLINE];//define buffer
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
    servaddr.sin_addr.s_addr = inet_addr(address);
    if (connect(sockfd, (struct sockaddr*)&servaddr,sizeof(servaddr)) < 0) {//lets try and connect to the server
        printf("\n Error : Connection to the Server Failed, Check that you entered the right address and port \n");
        return;
    }
    //clear the buffer
    memset(buffer, 0, sizeof(buffer));
    strcpy(buffer, "QOTD");//set the message to send to the server
     CaesarCipher(1, buffer);//encript the message that we are sending to the server
    send(sockfd, buffer, sizeof(buffer),0);//Send the Encripted message to the server
    memset(buffer, 0, MAXLINE);//clear the buffer
    read(sockfd, buffer, sizeof(buffer));//receive the tcp encripted message from the server
    CaesarCipher(2, buffer);//decript the message
    close(sockfd);//close the connection
    char counter[2];//this will be the line of the file that the qotd is on
    counter[0]=buffer[0];
    counter[1]=buffer[1];
    int numchoice;
    char QOTD[strlen(buffer-2)];
    for(int i=2;i<strlen(buffer)-2;i++){
        QOTD[i-2]=buffer[i];
    }
    printf("|-------------------------------------------------------------------------|\n");
    printf("| Quote Of The Day:%s\n",QOTD);//print the QOTD
    printf("|-------------------------------------------------------------------------|\n");
    do{
        printf("|-----------------------------------------------------|\n");
        printf("|                Quote Of The Day Menu                |\n");
        printf("|     Please Pick A Number From The Options Below     |\n");
        printf("|                                                     |\n");
        printf("|     1.) Select To Like The Quote Of The Day         |\n");
        printf("|     2.) Select To Dislike The Quote Of The Day      |\n");
        printf("|     3.) Select To Just Return To The Main Menu      |\n");
        printf("|-----------------------------------------------------|\n");
        printf("Option: ");
        scanf("%d", &numchoice);
        if(numchoice==1||numchoice==2||numchoice==3||numchoice==4){
            if(numchoice==1){//if the user selects to like the quote then we send the command
                char message[10];//create the message
                sprintf(message, "LIKE%s",counter);
               char output5[MAXLINE];
                sendandrecieve(sockfd, message, output5, servaddr);//send the command to the server and receive the output
                printf("%s\n",output5);
                numchoice=3;//set the selction to num3;
            }else if(numchoice==2){//if the user selects to dislike the qotd send that command to the server
                char message[MAXLINE];
                sprintf(message, "DISLIKE%s",counter);
                char output[MAXLINE];
                sendandrecieve(sockfd, message, output, servaddr);//send dislike to server
                printf("%s\n",output);
                numchoice=3;
            }else if(numchoice==4){//this is a hidden feture that we can send the server for testing to see the outputs of the dislike array
                char message[]="LISTALL";
                char output[MAXLINE];
                sendandrecieve(sockfd, message, output, servaddr);
                printf("%s\n",output);
                numchoice=3;
            }else{
                scanf("Please Enter One Of The Options");
            }
        }
    }while(numchoice !=3);
    //we exit the loop becuase the user has liked or disliked or selected to exit to the main menu
    //before we had to tell the server we where closing the connection but now we dont have to do that
    //sendandrecieve(sockfd, message, output, servaddr);
    //so just return
    return;
}
//-----------------------------------------------------------------------------------------------|
//                                Countryselectorhelper();                                       |
//       This Function Sends a TCP Connection to the server and receives the Information         |
//      If the code dosent exist in the database or an error the server will send that back      |
//-----------------------------------------------------------------------------------------------|
void Countryselectorhelper(int sel,int code,int PORT,char address[]){
    int sockfd;//define the socket
    struct sockaddr_in servaddr;//define the server struct
    // Creating socket file descriptor
    memset(&servaddr, 0, sizeof(servaddr));
    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr(address);
    //create the command we will send to the server
    char message[11];
    if(sel==1){
        sprintf(message, "IDCC%d",code);//for country code
        char output5[MAXLINE];
        sendandrecieve(sockfd, message, output5, servaddr);//we encript the command and send it to the server and get the output
         printf("%s\n",output5);//print the value
    }
    if(sel==2){
        sprintf(message, "IDCN%d",code);//for county name
        char output5[MAXLINE];
        sendandrecieve(sockfd, message, output5, servaddr);
         printf("%s\n",output5);//print the value
    }
    if(sel==3){
        sprintf(message, "IDCO%d",code);//for the countrys continate
        char output5[MAXLINE];
        sendandrecieve(sockfd, message, output5, servaddr);
        printf("%s\n",output5);//print the value
    }
    //after the command is sent and recieved return to the main menu
    //if an invalid command is sent to the method it will also just return
    return;
}
//-----------------------------------------------------------------------------------------------|
//                                      sendandrecieve();                                        |
//     This Function handles the encripting sending receiving and decripting of information      |
//      If the code dosent exist in the database or an error the server will send that back      |
//-----------------------------------------------------------------------------------------------|
void sendandrecieve(int sockfd,char input[],char output[],struct sockaddr_in servaddr){
    //create the socket and define it
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket creation failed");
        return;
    }
    //send a connect to the server
    if (connect(sockfd, (struct sockaddr*)&servaddr,
                sizeof(servaddr)) < 0) {//lets try and connect to the server
        printf("\n [Client]: Connection to the Server Failed, Check that you entered the right address and port \n");
        return;
    }
    CaesarCipher(1,input);//encript the packets to send
    write(sockfd, input, strlen(input));//send the tcp encripted message to the server
   ssize_t count = read(sockfd, output, MAXLINE);//receive the tcp encripted message from the server
    output[count] = '\0';
    CaesarCipher(2,output);//decript the message from the server
    close(sockfd);//close the connection
    return;//return to main
}
//end of Connection Helper Libary
