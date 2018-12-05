#include <stdio.h>      	/* for printf() and fprintf() */
#include <stdlib.h>     	/* for atoi() and exit() */
#ifdef _WINDOWS                 /* IF ON A WINDOWS PLATFORM YOU WILL HAVE TO CHECK THIS */
#include <winsock2.h>           /* IF ON A WINDOWS PLATFORM YOU WILL HAVE TO CHECK THIS */
#include <ws2tcpip.h>           /* IF ON A WINDOWS PLATFORM YOU WILL HAVE TO CHECK THIS */
#else
#include <arpa/inet.h>  	/* for sockaddr_in and inet_ntoa() */
#include <sys/socket.h> 	/* for socket(), bind(), and connect() */
#include <string.h>     	/* for memset() */
#include <unistd.h>     	/* for close() */
#include <netinet/in.h>  	/* for IVP6 and SCTP support       */
#include <netinet/sctp.h>  	/* for SCTP support       */
#include <netdb.h>     /* for gethostbyname() */
#endif

void DieWithError(char *errorMessage);  /* Error handling function */
static void print_src(int , sctp_assoc_t ); //this function takes in the struct of the information we get from the sender and turns it into a usabull printable server address and port

int main(int argc, char *argv[])
{
    int sock_Descr,ret;                  /* Socket descriptor */
    struct sockaddr_in echoServAddr; /* Echo server address */
    unsigned short echoServPort;     /* Echo server port */
    char *servIP;                    /* Server IP address (dotted quad) */
    char *echoString;                /* String to send to echo server */
    unsigned int echoStringLen;      /* Length of string to echo */
    int totalBytesRcvd;    	     /* Bytes read in single recv() 
                                        and total bytes read */
    char hostipaddr[INET_ADDRSTRLEN];
//------------------------------------
 //This is where we setup our SCTP Flags and events
    struct sctp_event_subscribe sctpEventSub;//receive event when it is established, when it is teared down, remote errors, etc.
    bzero(&sctpEventSub, sizeof(sctpEventSub));//zero the structure
    sctpEventSub.sctp_data_io_event = 1;//set the structure Eventsub to have the event Data IO Event
    struct sctp_sndrcvinfo sndrInfo; //structure to get the senders information
    socklen_t servLen;//the servers length
    ssize_t sendSize;//the send size of the message from the sevrer
    int msgFlags = 0;//the flags included
//------------------------------------


    /* ------Step 0 check user input ------ */
    /* Test for correct number of arguments */
    if ((argc < 3) || (argc > 4))    /* Test for correct number of arguments */
    {
       fprintf(stderr, "Usage: %s <Server IP> <Echo Word> [<Echo Port>]\n",
               argv[0]);
       exit(1);//exit the program
    }

    servIP = argv[1];             /* First arg: server IP address (dotted quad) */
    echoString = argv[2];         /* Second arg: string to echo */

    if (argc == 4)
        echoServPort = atoi(argv[3]); /* Use given port, if any */
    else
        echoServPort = 7;  /* 7 is the well-known port for the echo service */

    /* ------Step 1 create socket --------------- */
    if ((sock_Descr = socket(PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP)) < 0)
        DieWithError("socket() failed");

//------------------------------------
// Lab 7 Part B - comment this section
    //first we create a socket with the protacall SCTP with our event structure from before allowing it to know what events we are looking for
    if (setsockopt(sock_Descr, IPPROTO_SCTP, SCTP_EVENTS, &sctpEventSub, sizeof(sctpEventSub)) < 0 )//set the size
        DieWithError("setsocketopt() SCTP_Events failed");//if failed goto ERROR

//------------------------------------
    
    
    //IMPORTANT
    //not sure why you are hsveing us use the old verizon of the call its not good programing to use a deprived function
    //for the safty and good coding practice i am going to use the new function and use it in a way that i fit works the best
    //if you do it this way it can be motified to accept BOTH IPV4 and IPV6 so you can make it simple on yourslef
    /* Construct the server address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));     /* Zero out structure */
    echoServAddr.sin_family      = AF_INET;             /* Internet address family */
    struct addrinfo hints, *infoptr; //our struct to find the hostname
    hints.ai_family = AF_INET; //only searching dns for ipv4 addresses
    if (inet_pton(AF_INET, servIP, &echoServAddr.sin_addr) > 0) {
        printf("Found a user Defined IPV4 Address\n");
        //note inet_pton will take care of setting the address
    }else{//if the address isnt ipv6 we look for it in the dns
        printf("Found a Hostname Searching For The Address Now...\n");
        int result = getaddrinfo(servIP, NULL, &hints, &infoptr);
        if (result) {
            fprintf(stderr, "Failed to find the ipv4 hostname make sure you inputed the address or hostname correctly %s\n", gai_strerror(result));
            exit(1);
        }
        char host[INET_ADDRSTRLEN];//create the string to hold the address
        getnameinfo(infoptr->ai_addr, infoptr->ai_addrlen, host, sizeof (host), NULL, 0, NI_NUMERICHOST);//serch dns for entry matching name
        inet_pton(AF_INET, host, &echoServAddr.sin_addr);//copy the address found into the structure
        sprintf(hostipaddr,"%s",host);
        printf("Found The IPV4 Address Of %s [%s]\n",servIP,host);
    }
    echoServAddr.sin_port = htons(echoServPort); /* Server port */
    echoStringLen = strlen(echoString);                 /* Determine input length */

    /* ------Step 2 send message to server ------- */
    /* Send the string to the server */
//------------------------------------
// Lab 7 Part B - comment this section - what are the fields for?
    //i added the Connect TO make the program work easier on me
    ret = connect (sock_Descr, (struct sockaddr *) &echoServAddr, sizeof (echoServAddr));
    //first we send a connect to the server so it knows that we are trying to connect
    servLen = sizeof(echoServAddr);//set the size of the server length from the structure of our server address
    bzero(&sndrInfo, sizeof(sndrInfo));//zero out the streucture
    //when we send the messsage to the server we have to check if the message that sent acuall is equal to the total length of the message we are trying to send its an easy way of making sure that the msssage we send acually works as intended
    if ((sendSize = sctp_sendmsg(sock_Descr,
                    echoString,//sending the string
                    echoStringLen,//sending string length
                    (struct sockaddr *) &echoServAddr,//the server address streucture holding all the information on the server
                    servLen,//the length of the server
                    0, 0, //the SCTP Flaga we will be sending
                    sndrInfo.sinfo_stream,//the type of SCTp we want to send
                    0,0            )) != echoStringLen)
        DieWithError("send() sent a different number of bytes than expected");//quit to error
    else
        printf("Sent to the Server: [%.*s]\n",echoStringLen, echoString);      /* Print the echo buffer */
//------------------------------------
        
    /* ------Step 3 recv message from server ------ */
    /* Receive the same string back from the server */
//------------------------------------
// Lab 7 Part B - comment this section - what are the fields for?
    if ((totalBytesRcvd = sctp_recvmsg(sock_Descr,//the SCTp Recev allows us to get what was sent to us from the server
                          echoString,//the string we are receaving
                          sizeof(echoStringLen),//the length of the string
                          (struct sockaddr *) &echoServAddr,//the Structure of the Server
                          &servLen,//The length of the server structure
                          &sndrInfo,//the information on the sender that we keep in our structure from earilyer
                          &msgFlags)) < 0 )//the flags sent to us
        DieWithError("recv() had an error ");//if error quit
    else {
        print_src(sock_Descr, sndrInfo.sinfo_assoc_id);//run the method to parse the information on the server who sent us the things
        printf("Recvd from Server: [%.*s]\n",echoStringLen, echoString);      /* Print the echo buffer */
    }
//------------------------------------


    /* ------Step 4 close connection with server and release resources ------ */
//------------------------------------
// Lab 7 Part B - comment this section - what behavior do we expect from the shutdown?
    shutdown(sock_Descr,SHUT_WR);
//------------------------------------

#ifdef _WINDOWS			/* IF ON A WINDOWS PLATFORM YOU WILL HAVE TO CHECK THIS */
    WSACleanup()
#endif
    exit(0);
}

//------------------------------------
// Lab 7 Part B - comment this section - What is this function for ?
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
        printf("Msg from Server on association - %d IP:Port - %s:%d\n", assoc_id, tmpname, port);//print the message of who sent us the data
}
//------------------------------------
