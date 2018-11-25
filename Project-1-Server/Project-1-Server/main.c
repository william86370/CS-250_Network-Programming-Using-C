// Server program
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>           //for time
#include <fcntl.h> /* Added for the nonblocking socket */
#include <sys/wait.h>
#define PORT 5000
#define MAXLINE 1024
#define BST (+1) //time in diffrent zone
#define CCT (+8) //time in diffrent zone
int max(int x, int y)
{
    if (x > y)
        return x;
    else
        return y;
}
void lookupid(int countryid,char cname[80],char countrycode[3],char ccon[3]);
int CaesarCipher(int option, char str[]);
void writepipe(int fd,char input[],int len);
void readpipe(int fd,char output[MAXLINE]);
int array_to_num(int arr[],int n);
void sendmessage(int sockfd,char input[],int len);
void sendandrecieve(int sockfd,char input[],char output[]);
int main()
{
    //file io setup
    
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
    
    int reactionarraylike[100] = { 0 };;
    int reactionarraydislike[100] = { 0 };;
    
    
    int listenfd, connfd, udpfd, nready, maxfdp1;
    char buffer[MAXLINE];
    pid_t childpid;
    fd_set rset;
    ssize_t n;
    socklen_t len;
    const int on = 1;
    struct sockaddr_in cliaddr, servaddr;
    
    void sig_chld(int);
    //setup the time settings for udp
    time_t rawtime;
    struct tm *info;
    time(&rawtime);
    /* Get GMT time */
    info = gmtime(&rawtime );
    
    printf("|------------------------------------------------------------------------------------------|\n");
    printf("|                              CS-250 Project-1 UDP-TCP Server                             |\n");
    printf("|                                Developed By William Wright                               |\n");
    printf("|     When Launched The Server will bind to port 5000 on TCP and open a Listener on UDP    |\n");
    printf("|     The Server Will Spawn New Childs For Each Connection And Kill Them On Disconnect     |\n");
    printf("|------------------------------------------------------------------------------------------|\n");
    printf("Setting Up Connection....\n");
    char * sendtime = "nah";
    /* create listening TCP socket */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
    
    // binding server addr structure to listenfd
    bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    listen(listenfd, 20);
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
    for (;;) {
        //added piping so child could talk to parent
        int fd[2];
        if ( pipe(fd) < 0 ) {
            perror( "pipe" );
        }
        printf("--------------------------\n");
        printf("Waiting For Connection....\n");
        // set listenfd and udpfd in readset
        FD_SET(listenfd, &rset);
        FD_SET(udpfd, &rset);
        
        // select the ready descriptor
        nready = select(maxfdp1, &rset, NULL, NULL, NULL);
        
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
                read(connfd, buffer, sizeof(buffer));
                CaesarCipher(2,buffer);
                writepipe(fd[1], buffer,sizeof(buffer));
                if (strncmp(buffer, "LIKE", 4) == 0) {
                    int count3=0;
                     sscanf(buffer, "LIKE%d", &count3);
                    printf("the line is%d was added\n",count3);
                    reactionarraylike[count3]++;
                    printf("%s Liked the Quote Of the Day\n",inet_ntoa(cliaddr.sin_addr));
                    char input[]="Liked Message";
                    sendmessage(connfd,input,strlen(input));
                    shutdown(connfd,0);
                    _Exit(4);
                }
                if (strncmp(buffer, "DISLIKE", 7) == 0) {
                    int count3=0;
                     sscanf(buffer, "DISLIKE%d", &count3);
                    printf("the line %d was disliked\n",count3);
                    reactionarraydislike[count3]++;
                    printf("%s Disliked the Quote Of the Day\n",inet_ntoa(cliaddr.sin_addr));
                    char input[]="Disliked Message";
                    sendmessage(connfd,input,strlen(input));
                    shutdown(connfd,0);
                    exit(1);
                }
                if(strcmp(buffer,"EXIT") == 0){
                    printf("\nDisconnected From %s \n",inet_ntoa(cliaddr.sin_addr));
                    char input[]="Killing";
                    sendmessage(connfd,input,strlen(input));
                    shutdown(connfd,0);
                    exit(1);
                }
                if(strcmp(buffer,"LISTALL") == 0){
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
                if(strcmp(buffer,"QOTD") == 0){
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
                    }while(reactionarraydislike[num]==1);
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
                                CaesarCipher(1,Output);
                                write(connfd,(const char*)Output, sizeof(buffer));
                                shutdown(connfd,0);
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
                        fclose(file);
                    }
                    else
                    {
                        //file doesn't exist
                        close(connfd);
                        exit(1);
                    }
                }
                //get county code from ID
                if (strncmp(buffer, "IDCC", 4) == 0) {
                    char countryid[MAXLINE] ;
                    sscanf(buffer, "IDCC%s", &countryid);
                    char line[256];
                    bzero(line, sizeof(line));
                    char filename[] = "Country_DB.csv";
                    FILE *file = fopen(filename, "r");
                    if ( file != NULL )
                    {
                        while (fgets(line, sizeof line, file) != NULL)
                        {
                            if (strncmp(line,countryid, 6) == 0) {
                                int cid=0;
                                char cname[80];
                                char countrycode[3];
                                char ccon[3];
                                sscanf(line,"%d,%2[^,],%80[^,],%2[^,]",&cid,countrycode,cname,ccon);
                                printf("Sent Country Code: %s : to %s\n",countrycode,inet_ntoa(cliaddr.sin_addr));
                                sendmessage(connfd,countrycode,strlen(countrycode));
                                printf("Disconnected From %s \n",inet_ntoa(cliaddr.sin_addr));
                                exit(1);
                                
                            }
                        }
                        fclose(file);
                    }
                    else
                    {
                        //file doesn't exist
                        close(connfd);
                        exit(1);
                    }
                    
                }
                //end
                if (strncmp(buffer, "IDCO", 4) == 0) {
                    char countryid[MAXLINE] ;
                    sscanf(buffer, "IDCO%s", &countryid);
                    char line[256];
                    bzero(line, sizeof(line));
                    char filename[] = "Country_DB.csv";
                    FILE *file = fopen(filename, "r");
                    if ( file != NULL )
                    {
                        while (fgets(line, sizeof line, file) != NULL)
                        {
                            if (strncmp(line,countryid, 6) == 0) {
                                int cid=0;
                                char cname[80];
                                char countrycode[3];
                                char ccon[3];
                                sscanf(line,"%d,%2[^,],%80[^,],%2[^,]",&cid,countrycode,cname,ccon);
                                printf("Sent Country Continante: %s : to %s\n",ccon,inet_ntoa(cliaddr.sin_addr));
                                sendmessage(connfd,ccon,strlen(ccon));
                                printf("Disconnected From %s \n",inet_ntoa(cliaddr.sin_addr));
                                exit(1);
                            }
                        }
                        fclose(file);
                    }
                    else
                    {
                        //file doesn't exist
                        close(connfd);
                        exit(1);
                    }
                }
                //get name from id
                if (strncmp(buffer, "IDCN", 4) == 0) {
                    char countryid[MAXLINE] ;
                    sscanf(buffer, "IDCN%s", &countryid);
                    char line[256];
                    bzero(line, sizeof(line));
                    char filename[] = "Country_DB.csv";
                    FILE *file = fopen(filename, "r");
                    if ( file != NULL )
                    {
                        while (fgets(line, sizeof line, file) != NULL)
                        {
                            if (strncmp(line,countryid, 6) == 0) {
                                int cid=0;
                                char cname[80];
                                char countrycode[3];
                                char ccon[3];
                                sscanf(line,"%d,%2[^,],%80[^,],%2[^,]",&cid,countrycode,cname,ccon);
                                 printf("Sent Country Name: %s : to %s\n",countrycode,inet_ntoa(cliaddr.sin_addr));
                                sendmessage(connfd,cname,strlen(cname));
                                printf("Disconnected From %s \n",inet_ntoa(cliaddr.sin_addr));
                                exit(1);
                                
                            }
                        }
                        fclose(file);
                    }
                    else
                    {
                        //file doesn't exist
                        close(connfd);
                        exit(1);
                    }
                    
                }
                shutdown(connfd,0);
                exit(1);
            }
            //parent handleing
            //close our lissiner and write pipe
            close(connfd);
            close(fd[1]);
            //read the pipe from the child
            char buff[MAXLINE];
            readpipe(fd[0], buff);
            if (strncmp(buff, "LIKE", 4) == 0) {
                int count3=0;
                sscanf(buff, "LIKE%d", &count3);
                reactionarraylike[count3]++;
            }
            if (strncmp(buff, "DISLIKE", 7) == 0) {
                int count3=0;
                sscanf(buff, "DISLIKE%d", &count3);
                reactionarraydislike[count3]++;
            }
            //here we wait for the child tcp connnection to finnish
            int status;
            waitpid(childpid, &status, 0);
        }
        // if udp socket is readable receive the message.
        if (FD_ISSET(udpfd, &rset)) {
            bzero(echoBuffer, sizeof(echoBuffer));
            len = sizeof(cliaddr);
            if ((rcvMsgSize = recvfrom(udpfd, echoBuffer, sizeof(echoBuffer), 0,(struct sockaddr*)&cliaddr, &len)) < 0)
                printf("Found New UDP Connection\n");
            printf("Connecting To: %s\n",inet_ntoa(cliaddr.sin_addr));
            if(strcmp(echoBuffer,"GMT") == 0){
                printf("%s is Requesting GMT Time\n",inet_ntoa(cliaddr.sin_addr));
                time(&rawtime);
                /* Get GMT time */
                info = gmtime(&rawtime );
                asprintf(&sendtime,"GMT Time: [%d/%d/%d %d:%d:%d]",info->tm_mon + 1,info->tm_mday, info->tm_year + 1900, (info->tm_hour)%24, info->tm_min, info->tm_sec);
            }
            if(strcmp(echoBuffer,"Local") == 0){
                printf("%s is Requesting Local Time\n",inet_ntoa(cliaddr.sin_addr));
                time(&rawtime);
                /* Get GMT time */
                info = gmtime(&rawtime );
                asprintf(&sendtime, "Local Time: [%d/%d/%d %d:%d:%d]",info->tm_mon + 1,info->tm_mday, info->tm_year + 1900, ((info->tm_hour)%24) -5, info->tm_min, info->tm_sec);
            }
            rcvMsgSize = strlen(sendtime);
            if (sendto(udpfd,sendtime, rcvMsgSize, 0,
                       (struct sockaddr*)&cliaddr, sizeof(cliaddr)) != rcvMsgSize);
            else {
                printf("Sent Client %s The Time: [%s]\n",inet_ntoa(cliaddr.sin_addr), sendtime);
            }
            printf("Disconnected From %s \n",inet_ntoa(cliaddr.sin_addr));
            
        }
        
    }
}
void sendandrecieve(int sockfd,char input[],char output[]){
    memset(output, 0, sizeof(MAXLINE));
    read(sockfd, output, MAXLINE);//send the tcp encripted message to the server
    CaesarCipher(2,output);
    CaesarCipher(1,input);
    write(sockfd, input, MAXLINE);//receive the tcp encripted message from the server
    close(sockfd);
    return;
}
void sendmessage(int sockfd,char input[],int len){
    CaesarCipher(1,input);
    write(sockfd, input, len);//receive the tcp encripted message from the server
    close(sockfd);
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
    write(fd,input,len);
    return;
}
void readpipe(int fd,char output[MAXLINE]){
    ssize_t count = read(fd,output,MAXLINE-1 );
    if ( count <= 0 ) {
        perror( "read" );
        return;
    }
    output[count] = '\0';
    return;
}
void lookupid(int countryid,char cname[80],char countrycode[3],char ccon[3]){
    char line[256];
    bzero(line, sizeof(line));
    char filename[] = "Country_DB.csv";
    FILE *file = fopen(filename, "r");
    if ( file != NULL )
    {
        while (fgets(line, sizeof line, file) != NULL) /* read a line */
        {
            if (strncmp(line,countryid , 6) == 0) {
                int cid =0;
                sscanf(line,"%d,%2[^,],%80[^,],%2[^,]",&cid,countrycode,cname,ccon);
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
