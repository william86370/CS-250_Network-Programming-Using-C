#include <stdio.h>      	/* for printf() and fprintf() */
#include <stdlib.h>     	/* for atoi() and exit() */
#ifdef _WIN32                   /* IF ON A WINDOWS PLATFORM YOU WILL HAVE TO CHECK THIS */
#include <winsock2.h>           /* IF ON A WINDOWS PLATFORM YOU WILL HAVE TO CHECK THIS */
#include <ws2tcpip.h>           /* IF ON A WINDOWS PLATFORM YOU WILL HAVE TO CHECK THIS */
#else
#include <arpa/inet.h>  	/* for sockaddr_in and inet_ntoa() */
#include <sys/socket.h> 	/* for socket(), bind(), and connect() */
#include <string.h>     	/* for memset() */
#include <unistd.h>     	/* for close() */
#include <netinet/in.h>  	/* for IPV6,  SCTP support         */
#include <netinet/sctp.h>  	/* for SCTP support       */
#endif

#define MAXBUFF		80	
#define MAXPENDING	5

void DieWithError(char *errorMessage);  /* Error handling helper function */
static void print_src(int , sctp_assoc_t ); //this function takes in the struct of the information we get from the sender and turns it into a usabull printable server address and port

int main(int argc, char *argv[])
{
    int servSock_d;                     /* Socket descriptor for server */
    struct sockaddr_in echoServAddr;    /* Local address */
    struct sockaddr_in echoClntAddr;    /* Client address */
    unsigned short echoServPort;        /* Server port */

//------------------------------------
//Lab7 Part B - What are these fields for?
    char readBuffer[MAXBUFF-1]; //this is the buffre that the sever will be using to lead the message sent frommthe client the reason we use MAXBUFF is becuase we dont know the length of the message the client is sending but we do have a max value that both have agreeed on
    struct sctp_event_subscribe sctpEventSub; //receive event when it is established, when it is teared down, remote errors, etc.
    bzero(&sctpEventSub, sizeof(sctpEventSub));//we zero the structure
    sctpEventSub.sctp_data_io_event = 1; //set the value of IO becuase we want to log the IO data

    struct sctp_sndrcvinfo sndrInfo; //a structure to contain the senders information
    socklen_t clntLen;//the length of the client connecting
    ssize_t readSize;//the readsize of the message
    int reUse = 1;//the flags
    int maxIdle = 1;//the flags
    int msgFlags;//the total flags we will have
    
//------------------------------------

    /* ------Step 0 check user input ------ */
    /* Test for correct number of arguments from user */
    if (argc != 2) {     
        fprintf(stderr, "Usage:  %s <Server Port>\n", argv[0]);
        exit(1);
    }

    echoServPort = atoi(argv[1]);  	/* First arg:  should be local port */

    /* ------Step 1 create the socket ------- */
    /* Create socket for incoming connections */
    if ((servSock_d = socket(PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP)) < 0)
        DieWithError("socket() failed");
      
    /* Construct local address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    echoServAddr.sin_port = htons(echoServPort);      /* Local port */

    /* ------Step 2 bind the connection ip:port ------- */
    /* Bind to the local address */
    if (bind(servSock_d, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("bind() failed");

//------------------------------------
//Lab7 Part B - Why are we setting the following options and what arew we setting them to?
    //The socket will now be marked as reusable. for the program to keep using
    if (setsockopt(servSock_d, SOL_SOCKET, SO_REUSEADDR, &reUse, sizeof(reUse)) < 0 )
        DieWithError("setsocketopt() SO_REUSEADDR failed");
    //SCTP_AutoClose allows the sevrer to autoclose the Socket we are using if it detects that the client has disconnected
    if (setsockopt(servSock_d, IPPROTO_SCTP, SCTP_AUTOCLOSE, &maxIdle, sizeof(maxIdle)) < 0 )
        DieWithError("setsocketopt() SCTP_AUTOCLOSE failed");
    //the SCTP_events tells the socket that we will be lissiening with the Events Turned on to log information
    if (setsockopt(servSock_d, IPPROTO_SCTP, SCTP_EVENTS, &sctpEventSub, sizeof(sctpEventSub)) < 0 )
        DieWithError("setsocketopt() SCTP_Events failed");

//------------------------------------

    /* ------Step 3 listen to the socket for a connection ------- */
    /* Establish the socket to listen for incoming connections */
    if (listen(servSock_d, MAXPENDING) < 0)
        DieWithError("listen() failed");

    for (;;) /* Infinite Loop - run until process killed */
    {
        /* Set the size of the in-out parameter */
       clntLen = sizeof (struct sockaddr_in);

    /* ------Step 4 recv from the client ------- */
//------------------------------------
//Lab7 Part B - Why are the fields and what are we doing?
        //when we recieve the message using SCTp there is much that we need todo
       readSize = sctp_recvmsg(servSock_d,//set the socket we are receviing with
                               readBuffer,//the buffrer to hold the mssaage
                               sizeof(readBuffer),//the size of the read buffer
                               (struct sockaddr *) &echoClntAddr,//the client Structure to copy the info into
                               &clntLen,//the length of the client streucture
                               &sndrInfo,//the structre to copy the SCTP Sender info into
                               &msgFlags);//the flags that the client has sent to the server
       if (readSize > 0) {
          print_src(servSock_d, sndrInfo.sinfo_assoc_id);
          printf("Msg recvd from client: [%.*s]\n",(int) readSize, readBuffer );
       } 
//------------------------------------


    /* ------Step 5 send to the client --------- */
//------------------------------------
//Lab7 Part B - Why are the fields and what are we doing?
       sctp_sendmsg(servSock_d,
                    readBuffer,
                    readSize, 
                    (struct sockaddr *) &echoClntAddr,
                    clntLen,
                    sndrInfo.sinfo_ppid,
                    sndrInfo.sinfo_flags,
                    sndrInfo.sinfo_stream,
                    0,0            );
    } 
//------------------------------------
    /* ------Step 5+ implied close when program is terminated ------- */
}

//------------------------------------
//Lab7 Part B - Comments should be the same for here as in EchoClientMto1SCTP.c just copy here 
static void print_src(int fd, sctp_assoc_t assoc_id)//this funcion takes the SCTp structure containing the information on the Sender
{
    // Lab 7 Part B - comment this section - What are the fields for?
    struct sctp_status sstat; //recreate the structure
    struct sctp_paddrinfo *spinfo; //pointer to a sctp_paddrinfo struct
    char tmpname[INET_ADDRSTRLEN];//the tempname string with the size of a IPV4 Address
    unsigned int port;//the port the sender used
    unsigned int ulen;//the length of the sended
    struct sockaddr_in *s_in; //a pointer to assembele data
    
    bzero(&sstat, sizeof (sstat));//zero the structure
    
    ulen = sizeof (sstat);//set the size in memory
    // Lab 7 Part B - comment this section - What values are we interested in returning?
    if (sctp_opt_info(fd, assoc_id, SCTP_STATUS, &sstat, &ulen) < 0) {//this function takes int the input struct from the sender and changes the value of the structs defined eariler to allow for the data from the sender
        perror("sctp_opt_info()");
        return;
    }
    spinfo = &sstat.sstat_primary;//the primary sendiner
    
    // Lab 7 Part B - comment this section - What are we doing on each line?
    s_in = (struct sockaddr_in *) &spinfo->spinfo_address;//geting the IPV4 Address in binarry notation
    inet_ntop(AF_INET, &s_in->sin_addr, tmpname, sizeof (tmpname));//changeing the IPV4 into a readable format for us to print
    port = ntohs(s_in->sin_port);//converting the port from binary to Readable
    printf("Msg from Client on association - %d IP:Port - %s:%d\n", assoc_id, tmpname, port);//print the message of who sent us the data
}
//------------------------------------

