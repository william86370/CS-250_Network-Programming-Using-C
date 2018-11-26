//
//  Main.c
//  CS-250-Project-1
//
//  Created by william wright on 11/15/18.
//  Copyright © 2018 Silvertap. All rights reserved.
//
#include <arpa/inet.h>//server
#include <errno.h>//server
#include <netinet/in.h>//server
#include <signal.h>//added for the signal handleing "dont need"
#include <stdio.h>//default libary
#include <stdlib.h>//default libary
#include <strings.h>//added for our beutifull strings
#include <sys/socket.h>//added for our tcp and udp socket
#include <sys/types.h>//server
#include <unistd.h>//default libary
#include <time.h> //for time
#include <fcntl.h> // Added for the nonblocking socket
#include <sys/wait.h>//added for parent waiting for child
#define MAXLINE 1024//the maxsize of our buffer
//these are defined for maing time easier to handle in local values
#define BST (+1) //time in diffrent zone
#define CCT (+8) //time in diffrent zone

//---------------------------------------------See Methods Below For More Information-----------------------------------------------------|
void Setup(int config[3]);//for port setup and settings                                                                              |
int max(int x, int y);//for handleing the TCP VS UDP Packets                                                                          |
void lookupid(int countryid,char cname[80],char countrycode[3],char ccon[3]);//this handles the lookup of the Country ID's |
int CaesarCipher(int option, char str[]);//this is our encription for sending and recieveing TCP                                 |
void writepipe(int fd,char input[],int len); //this lets us communicste between parent and child                                 |
void readpipe(int fd,char output[MAXLINE]); //this lets the parent recieve what the child is sending                               |
int array_to_num(int arr[],int n); //undefined                                                                                     |
void sendmessage(int sockfd,char input[],int len);//this allows our TCP Child to send a Packet to the Client                    |
void sendandrecieve(int sockfd,char input[],char output[]);//allows us to recieve and send packets faster                     |
//---------------------------------------------See Methods Below For More Information-----------------------------------------------------|

int main()
{
    printf("|------------------------------------------------------------------------------------------|\n");
    printf("|                              CS-250 Project-1 UDP-TCP Server                             |\n");
    printf("|                                Developed By William Wright                               |\n");
    printf("|       When Launched The Server will bind to The User Defined Port on TCP and UDP         |\n");
    printf("|     The Server Will Spawn New Childs For Each Connection And Kill Them On Disconnect     |\n");
    printf("|------------------------------------------------------------------------------------------|\n");
    //-----------------------------------------------------------------------------------------------|
    //Setup Of the Server Here We check for the files and Also Load/Create a Config For Settings     |
    //-----------------------------------------------------------------------------------------------|
    int config[3];//the settings array
    Setup(config);//the function that handles setup of the server
    int PORT = config[0];//port the server will run on
    int numdislikes=config[1];//number of dislikes to exclude that line from the QOTD
    int clients = config[2]; //number of CLients that can connect at a time to the server
    //-----------------------------------------------------------------------------------------------|
    //here we open both the quote of the day file and the Country database file to see if they exist.|
    //we also get a coiunter from the QOTD file to see how long it is and save that to Count2        |
    //-----------------------------------------------------------------------------------------------|
    FILE *fp;//our file
    int count2 = 0;  // Line counter (result)
    char filename[] = "Quote_File.txt";
    char c;  // To store a character read from file
    // Open the file
    fp = fopen(filename, "r");
    // Check if file exists
    if (fp == NULL)
    {
        printf("Could not open file %s", filename);
        return 0;
    }
    // Extract characters from file and store in character c
    for (c = getc(fp); c != EOF; c = getc(fp))
        if (c == '\n') // Increment count if this character is newline
            count2 = count2 + 1;
    // Close the file
    fclose(fp);
    //----------------------------------------------------------------------------------|
     //here we create the arrays that are going to be filled with the likes and dislikes|
    //----------------------------------------------------------------------------------|
    int reactionarraylike[100] = { 0 };;
    int reactionarraydislike[100] = { 0 };;
    //------------------------------------------------------------|
    //here we setup the varables for the lissiner for TCP And UDP |
    //------------------------------------------------------------|
    int listenfd, connfd, udpfd, nready, maxfdp1;
    char buffer[MAXLINE];
    pid_t childpid;
    fd_set rset;
    ssize_t n;
    socklen_t len;
    const int on = 1;
    struct sockaddr_in cliaddr, servaddr;
    void sig_chld(int);
    //---------------------------------------------------------------------|
    //This is where we setup the random number generator for our UDP Return|
    //---------------------------------------------------------------------|
    time_t rawtime;
    struct tm *info;
    time(&rawtime);
    /* Get GMT time */
    info = gmtime(&rawtime);
    //print server time to Screen
    printf("Server Time: [%d/%d/%d %d:%d:%d]\n",info->tm_mon + 1,info->tm_mday, info->tm_year + 1900, ((info->tm_hour)%24) -5, info->tm_min, info->tm_sec);
    //------------------------------------------------------------|
    //We Create the Struct for UDP And TCP and assign a lissiner  |
    //------------------------------------------------------------|
   printf("Setting Up Connection....\n");
    char * sendtime = "[Server]:The Command You Sent Didnt match any of the commmands use /help to see all commands";
    /* create listening TCP socket */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
    // binding server addr structure to listenfd
    bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
//added clients to listen command
    listen(listenfd,clients);
    printf("TCP Deployment Sucessfull\n");
    /* create UDP socket */
    udpfd = socket(AF_INET, SOCK_DGRAM, 0);
    // binding server addr structure to udp sockfd
    bind(udpfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    printf("UDP Deployment Sucessfull\n");
    // clear the descriptor set
    FD_ZERO(&rset);
    unsigned short echoServPort;        /* Server port */
    char echoBuffer[MAXLINE];        /* Buffer for the echo string */
    int rcvMsgSize;            /* Size of the recieved message */
    // get maxfd
    maxfdp1 = max(listenfd, udpfd) + 1;
    int lastsentline= 0;
    //---------------------------------------------------------------------------------------------------|
    //here we create the for loop that allows the server to run infinadanlty untill the process is killed|
    //---------------------------------------------------------------------------------------------------|
    for (;;) {
        int fd[2]; //define our two pipes for talking
        if ( pipe(fd) < 0 ) {
            perror( "Couldnt Create pipe" );
            exit(1);
        }
        printf("--------------------------\n");
        printf("Waiting For Connection....\n");
        // set listenfd and udpfd in readset
        FD_SET(listenfd, &rset);
        FD_SET(udpfd, &rset);
        
        // select the ready descriptor
        nready = select(maxfdp1, &rset, NULL, NULL, NULL);
        
        //-----------------------------------------------------------------------------------------------------------|
        //                                        TCP Part of the Server                                             |
        //-----------------------------------------------------------------------------------------------------------|
        // if tcp socket is readable then handle
        // it by accepting the connection
        if (FD_ISSET(listenfd, &rset)) {
            len = sizeof(cliaddr);
            connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &len);
            fcntl(connfd, F_SETFL, O_NONBLOCK);
            if ((childpid = fork()) == 0) {
                close(listenfd);
                //close the other pipe
                close(fd[0]);
                bzero(buffer, sizeof(buffer));
                printf("Connected To: %s\n",inet_ntoa(cliaddr.sin_addr));
                read(connfd, buffer, sizeof(buffer));//Recieve the TCP Request From Client
                CaesarCipher(2,buffer);//Decript the TCP Packet
                writepipe(fd[1], buffer,sizeof(buffer));//Send a Copy of the decripted packet to parent for processing and logging
                if (strncmp(buffer, "LIKE", 4) == 0) { //if the message from client containded "like" we go into this IF allowing us to like the QOTD they recieved
                    int count3=0;
                    sscanf(buffer, "LIKE%d", &count3);
                    printf("the line is%d was added\n",count3);
                    reactionarraylike[count3]++;//update the array with the line they liked
                    printf("%s Liked the Quote Of the Day\n",inet_ntoa(cliaddr.sin_addr));
                    char input[]="Liked Message";
                    sendmessage(connfd,input,strlen(input));//our helper to send the message backto the client
                    shutdown(connfd,0);
                    _Exit(4);
                }
                if (strncmp(buffer, "DISLIKE", 7) == 0) {//if the message from client containded "Dislike" we go into this IF allowing us to dislike the QOTD they recieved
                    int count3=0;
                    sscanf(buffer, "DISLIKE%d", &count3);
                    printf("the line %d was disliked\n",count3);
                    reactionarraydislike[count3]++;//update the dislike array so it wont send that quote anymore
                    printf("%s Disliked the Quote Of the Day\n",inet_ntoa(cliaddr.sin_addr));
                    char input[]="Disliked Message";
                    sendmessage(connfd,input,strlen(input));//our helper to send the message backto the client
                    shutdown(connfd,0);
                    exit(1);
                }
                if(strcmp(buffer,"EXIT") == 0){//if the message from client containded "EXIT" we go into this IF allowing us to process that the client didnt Like Or dislike the QOTD
                    printf("\nDisconnected From %s \n",inet_ntoa(cliaddr.sin_addr));
                    char input[]="Killing";
                    sendmessage(connfd,input,strlen(input));//our helper to send the message backto the client
                    shutdown(connfd,0);
                    exit(1);
                }
                if(strcmp(buffer,"LISTALL") == 0){//this was a test function allowing us to print the arrays
                    printf("\nDisconnected From %s \n",inet_ntoa(cliaddr.sin_addr));
                    char input[count2];
                    for(int h=0;h<count2;h++){
                        printf("%d",reactionarraydislike[h]);
                        sprintf(input, "%s:%d", input, reactionarraydislike[h]);
                    }
                   // sendmessage(connfd,input);
                    shutdown(connfd,0);
                    exit(1);
                }
                if(strcmp(buffer,"QOTD") == 0){//when the server recived "QOTD" we open the file and selct a quote at random
                    printf("%s Is Requesting Quote Of the Day\n",inet_ntoa(cliaddr.sin_addr));
                    char line[256];
                    bzero(line, sizeof(line));
                    char filename[] = "Quote_File.txt";
                    int num=1;//our random number
                    time_t t;//random number timer
                    //we have a while loop to deal with the dislikes quotes and see if we are sending the user a disliked quote
                    do{
                    srand((unsigned) time(&t));
                    num = rand()%count2;
                    }while(reactionarraydislike[num]==numdislikes);
                    //when we have a quote that isnt disliked we send it to the client
                    FILE *file = fopen(filename, "r");
                    int count = 0;/* or other suitable maximum line size */
                    if ( file != NULL )
                    {
                        while (fgets(line, sizeof line, file) != NULL) /* read a line */
                        {
                            if (count == num)
                            {
                                char Output[MAXLINE];
                                sprintf(Output, "%02d%s", count, line);
                                CaesarCipher(1,Output);//encript the message to the client
                                write(connfd,(const char*)Output, sizeof(buffer));//send the message to the client
                                shutdown(connfd,0);//end all traffic
                                printf("Sent Quote Of the Day to %s\n",inet_ntoa(cliaddr.sin_addr));
                                printf("Disconnected From %s \n",inet_ntoa(cliaddr.sin_addr));
                                lastsentline = count;
                                exit(2);
                            }
                            else
                            {
                                count++;
                            }
                        }
                        //if the random number gives us a number of a line not in the file this will trigger
                        //so we close the file and return an error to the client
                        fclose(file);
                        printf("Error In Selecting The QOTD, Is the Disliked Array Full? Request From: %s\n",inet_ntoa(cliaddr.sin_addr));
                        char errormessage[] = "There Was An Error In Selecting The QOTD PLease Try Again Later";
                        sendmessage(connfd,errormessage,strlen(errormessage));
                        close(connfd);
                        exit(1);
                    }
                    else
                    {
                        //if the file was removed or dammaged while it is running this will return
                        //becuase we check the files at the begining this should never trigger
                        //so we close the file and return an error to the client
                        printf("Error In Selecting The QOTD, The File Dosent Exist anymore Check That Its There, Request From: %s\n",inet_ntoa(cliaddr.sin_addr));
                        char errormessage[] = "There Was An Error In Selecting The QOTD PLease Try Again Later";
                        sendmessage(connfd,errormessage,strlen(errormessage));
                        close(connfd);
                        exit(1);
                    }
                }
                //get county code from ID
                if (strncmp(buffer, "IDCC", 4) == 0||strncmp(buffer, "IDCO", 4) == 0||strncmp(buffer, "IDCN", 4) == 0) {
                    //this allows us to recieve the country id from the client and search the database for the line with the ID
                    //then we take apart the line and send it back to the server
                    char countryid[MAXLINE];
                    if (strncmp(buffer, "IDCO", 4) == 0) {
                        sscanf(buffer, "IDCO%s", &countryid);
                    }
                    if (strncmp(buffer, "IDCC", 4) == 0){
                        sscanf(buffer, "IDCC%s", &countryid);
                    }
                    if (strncmp(buffer, "IDCN", 4) == 0) {
                        sscanf(buffer, "IDCN%s", &countryid);
                    }
                    //after getting the Country ID we can open the File
                    char line[256];
                    bzero(line, sizeof(line));
                    char filename[] = "Country_DB.csv";
                    FILE *file = fopen(filename, "r");
                    if ( file != NULL )
                    {
                        while (fgets(line, sizeof line, file) != NULL)//read the file line by line
                        {
                            if (strncmp(line,countryid, 6) == 0) {//when the start of the line is equal to our ID we stop and parse
                                int cid=0;//the country ID
                                char cname[80];//the Country Name
                                char countrycode[3];//the Countrys Code
                                char ccon[3];//The Countrys Continante
                                sscanf(line,"%d,%2[^,],%80[^,],%2[^,]",&cid,countrycode,cname,ccon);//parse the line
                                if (strncmp(buffer, "IDCO", 4) == 0) {
                                     if (strcmp(ccon,"") == 0) {//if the ID dosent have an contanite then we tell the Client
                                         printf("%d :Dosent Have a contanite, Telling: %s\n",cid,inet_ntoa(cliaddr.sin_addr));
                                         char message[]="The Counrty ID You Requested Dosent Have a Contanite";
                                         sendmessage(connfd,message,strlen(message));
                                    }else{//if it dose have one we send it
                                        printf("Sent Country Continante: %s : to %s\n",ccon,inet_ntoa(cliaddr.sin_addr));
                                        sendmessage(connfd,ccon,strlen(ccon));
                                    }
                                }
                                if (strncmp(buffer, "IDCC", 4) == 0){
                                    printf("Sent Country Code: %s : to %s\n",countrycode,inet_ntoa(cliaddr.sin_addr));
                                    sendmessage(connfd,countrycode,strlen(countrycode));
                                }
                                if (strncmp(buffer, "IDCN", 4) == 0) {
                                    printf("Sent Country Name: %s : to %s\n",cname,inet_ntoa(cliaddr.sin_addr));
                                    sendmessage(connfd,cname,strlen(cname));
                                }
                                printf("Disconnected From %s \n",inet_ntoa(cliaddr.sin_addr));
                                exit(1);
                            }
                        }
                        //the country ID Didnt Exist in the file.
                        fclose(file);
                        printf("The Country ID: %s :Didnt Exist in the Database Sending Error Response to %s\n",countryid,inet_ntoa(cliaddr.sin_addr));
                        char errormessage[] = "[Server]:The Country ID You Provided Couldnt Be Located In The Database";
                        sendmessage(connfd,errormessage,strlen(errormessage));
                        close(connfd);
                        exit(1);
                    }
                    else
                    {
                        //the file couldnt be opened this shouldnt happen becuase we check that the file exists on startup but it is possable
                        printf("The Database File Dosent Exist Anymore Please Check By Relaunching The Server, Request From: %s\n",inet_ntoa(cliaddr.sin_addr));
                        char errormessage[] = "The Server Couldnt Open The Database PLease Try Again Later";
                        sendmessage(connfd,errormessage,strlen(errormessage));
                        close(connfd);
                        exit(1);
                    }
                }
                //this gets triggered when the client sends a command that isnt reconised by the sevrer
                printf("The Reueset From [%s:%s] Didnt Match Any Of The Server Commmands Handleing Error\n",inet_ntoa(cliaddr.sin_addr),buffer);
                char errormessage[] = "[Server]:The Command You Sent Didnt match any of the commmands use /help to see all commands";
                sendmessage(connfd,errormessage,strlen(errormessage));
                close(connfd);
                exit(1);
            }
            //-------------------------------------------------------|
            //While The Child IS handleing the connection we have to parrent recieve a copy of the message sent and process it
            //this allows us to handle memory better
            //close our lissiner and close the sending line for the pipe
            close(connfd);
            close(fd[1]);
            char buff[MAXLINE];//create the buffer
            readpipe(fd[0], buff);//read from our pipe to get the message that we sent to the server
            //becuase the child cant change the parents array we have to do it using the parrent
            //this makes the process easier and dont take up that much resouces
            if (strncmp(buff, "LIKE", 4) == 0) {//if the message sent is a LIKE we update the array
                int count3=0;
                sscanf(buff, "LIKE%d", &count3);
                reactionarraylike[count3]++;
            }
            if (strncmp(buff, "DISLIKE", 7) == 0) {//if the message sent is a dislike we update the array
                int count3=0;
                sscanf(buff, "DISLIKE%d", &count3);
                reactionarraydislike[count3]++;
            }
            //now that we have checked the message for any things the parent has to update we wait for the connection to be compleated and the child to exit
            int status;
            waitpid(childpid, &status, 0);//wainting for the child to exit
        }
        //-----------------------------------------------------------------------------------------------------------|
        //                                        UDP Part of the Server
        //-----------------------------------------------------------------------------------------------------------|
        //if the lissiner detects the incoming packet is UDP we handle that here
        if (FD_ISSET(udpfd, &rset)) {
            bzero(echoBuffer, sizeof(echoBuffer));//zero our buffer
            len = sizeof(cliaddr);
            if ((rcvMsgSize = recvfrom(udpfd, echoBuffer, sizeof(echoBuffer), 0,(struct sockaddr*)&cliaddr, &len)) < 0)
            printf("Found New UDP Connection\n");
            printf("Connecting To: %s\n",inet_ntoa(cliaddr.sin_addr));
            if(strcmp(echoBuffer,"GMT") == 0){//if the message we recieves has "GMT" as its command we go into this if
                printf("%s is Requesting GMT Time\n",inet_ntoa(cliaddr.sin_addr));
                time(&rawtime);
                info = gmtime(&rawtime);//get the current time in GMT
                asprintf(&sendtime,"GMT Time: [%d/%d/%d %d:%d:%d]",info->tm_mon + 1,info->tm_mday, info->tm_year + 1900, (info->tm_hour)%24, info->tm_min, info->tm_sec);//create our string to send back to the client
            }
            if(strcmp(echoBuffer,"Local") == 0){//if the message we recieved has "LOCAL" we get the GMT time and turn it into Local Time
                printf("%s is Requesting Local Time\n",inet_ntoa(cliaddr.sin_addr));
                time(&rawtime);
                /* Get GMT time */
                info = gmtime(&rawtime );
                asprintf(&sendtime, "Local Time: [%d/%d/%d %d:%d:%d]",info->tm_mon + 1,info->tm_mday, info->tm_year + 1900, ((info->tm_hour)%24) -5, info->tm_min, info->tm_sec);//create our string to send back to the client
            }
            rcvMsgSize = strlen(sendtime);//get the length on the string we are sedning
            if (sendto(udpfd,sendtime, rcvMsgSize, 0,(struct sockaddr*)&cliaddr, sizeof(cliaddr)) != rcvMsgSize)//send the client the responce
            printf("Sent Client %s The Responce: [%s]\n",inet_ntoa(cliaddr.sin_addr), sendtime);
            printf("Disconnected From %s \n",inet_ntoa(cliaddr.sin_addr));
        }
        //after the UDP Client Has Compleated We restart the for loop Looking for any new connections
    }
}
void sendandrecieve(int sockfd,char input[],char output[]){
    //Sockfd = the TCP Socket,input[] is a char array pointer, output[] is a char array pointer
    //this functions reads and decripts what is being sent over the socket
    //then replys by encripting the input and sending it over the socket
    memset(output, 0, sizeof(MAXLINE));
    read(sockfd, output, MAXLINE);//send the tcp encripted message to the server
    CaesarCipher(2,output);//decript the output
    CaesarCipher(1,input);//encript the input to send
    write(sockfd, input, MAXLINE);//send the encripted message back to the client
    close(sockfd);//close the socket
    return;
}
void sendmessage(int sockfd,char input[],int len){
    CaesarCipher(1,input);//encript the inout
    write(sockfd, input, len);//send the encripted message back to the client
    close(sockfd);//close the socket
}
int array_to_num(int arr[],int n){
    char str[2][2];
    int i;
    char number[13] = {'\n'};
    
    for(i=0;i<n;i++) sprintf(str[i],"%d",arr[i]);
    for(i=0;i<n;i++)strcat(number,str[i]);
    
    i = atoi(number);
     printf("%d:is the  umber returned",i);
    return i;
}
//added functions to allow talking between child and parent
void writepipe(int fd,char input[],int len){
    //this function takes in the pip for writing and an input and length
    write(fd,input,len);//send the input to the pipe
    return;//close the func
}
void readpipe(int fd,char output[MAXLINE]){
    //this function takes in the reading pipe and an output
    ssize_t count = read(fd,output,MAXLINE-1 );//read the message from child
    if ( count <= 0 ) {
        perror( "read Error For Parent PIPE" );
        return;
    }
    output[count] = '\0';//after receiving we set the length
    return;
}
void lookupid(int countryid,char cname[80],char countrycode[3],char ccon[3]){
    //this function isnt used anymore but added to test our ID
    char line[256];
    bzero(line, sizeof(line));
    char filename[] = "Country_DB.csv";
    FILE *file = fopen(filename, "r");
    if ( file != NULL )
    {
        while (fgets(line, sizeof line, file) != NULL) /* read a line */
        {
            if (strncmp(line,countryid , 6) == 0) {//check if line is equal to out ID
                int cid =0;
                sscanf(line,"%d,%2[^,],%80[^,],%2[^,]",&cid,countrycode,cname,ccon);//parse the line for the INFO
                return;
            }
        }
        fclose(file);
    }
    else
    {
        //file doesn't exist
        return;
    }
}
int max(int x, int y)
{
    if (x > y)
        return x;
    else
        return y;
}
void Setup(int config[3]){
    //the setup allows us to check for the required files prior to starting the server
    //we check for the config file the quote of the day file and the database file
    FILE *fp;//main file header
    //--------------------------------------------|
    //The three files that the server uses        |
    char filename[] = "Quote_File.txt";//     |
    char filename2[] = "Country_DB.csv";//    |
    char filename3[] = "ServerConfig.ini";//  |
    //--------------------------------------------|
    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("The %s File Could Not Be found The Sevrer will not start untill this file is in the starting directory\n", filename);
        exit(9);
    }
    printf("%s Exists\n",filename);
    fclose(fp);
    fp = fopen(filename2, "r");
    if (fp == NULL)
    {
        printf("The %s File Could Not Be found The Sevrer will not start untill this file is in the starting directory\n", filename2);
        exit(9);
    }
    fclose(fp);
    printf("%s Exists\n",filename2);
    fp = fopen(filename3, "r");
    if (fp == NULL)
    {
        //if the file dosent exit then we create it
        fclose(fp);
        printf("The Config File Couldnt be found begining First Time Setup\n");
        fp = fopen(filename3, "w+");
        int port;
        int count;
        int conn;
        printf("Enter The Port The Server Should Use For Connection: ");
        scanf("%d",&port);
        printf("\nEnter The Number Of Dislikes Until a Quote wont be Used: ");
        scanf("%d",&count);
        printf("\nEnter The Number Of Connections The Server Will Allow At A time: ");
        scanf("%d",&conn);
        fprintf(fp,"%d %d %d",port,count,conn);
        printf("\nConfig File Created\n");
        config[0]=port;
        config[1]=count;
        config[2]=conn;
        fclose(fp);
    }else{
        //the config file exists
        printf("Found Server Config File, Loading Values...\n");
        int port = 0;
        int count = 0;
        int conn = 0;
        fscanf(fp,"%d %d %d",&port,&count,&conn);
        if(port==0||count==0||conn==0){
            //if any of the values are 0 we set them to the defaults becuase having a value of zero isnt allowed
            config[0]=5000;//default value
            config[1]=5;//default value
            config[2]=20;//default value
            printf("The Config contained an Error, Setting Values to Default [Port:%d,Dislikes:%d,Clients:%d]\n",config[0],config[1],config[2]);
            fclose(fp);
            fp = fopen(filename3, "w+");
            fprintf(fp,"%d %d %d",config[0],config[1],config[2]);
        }else{
        config[0]=port;
        config[1]=count;
        config[2]=conn;
        printf("Loaded Server Config [Port:%d,Dislikes:%d,Clients:%d]\n",port,count,conn);
        }
        fclose(fp);
    }
    printf("Setup Complete All Files Ready...\n");
    printf("---------------------------------\n");
    return;
}
