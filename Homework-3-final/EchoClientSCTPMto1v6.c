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
#include <netdb.h>
#endif
void DieWithError(char *errorMessage);  /* Error handling function */
static void print_src(int , sctp_assoc_t ); //Lab7 Part B - What is this for??
int main(int argc, char *argv[])
{
    //------------------------------------------------------------------------------------
    int sock_Descr,ret;                  /* Socket descriptor */
    struct sockaddr_in6 echoServAddr; /* Echo server address */
    unsigned short echoServPort;     /* Echo server port */
    char *servIP;                    /* Server IP address (dotted quad) */
    char *echoString;                /* String to send to echo server */
    unsigned int echoStringLen;      /* Length of string to echo */
    int totalBytesRcvd;    	     /* Bytes read in single recv() and total bytes read */
    struct sctp_event_subscribe sctpEventSub;
    bzero(&sctpEventSub, sizeof(sctpEventSub));
    sctpEventSub.sctp_data_io_event = 1;
    struct sctp_sndrcvinfo sndrInfo;
    socklen_t servLen;
    ssize_t sendSize;
    char hostipaddr[INET6_ADDRSTRLEN];
    int msgFlags = 0;//int for storeing amount of flags
    int userport = 0;//for seeing is user entered custom port
    //------------------------------------------------------------------------------------
    printf("SCTP Client Developed By William Wright\nStarting Client...\n");
    /* ------Step 0 check user input ------ */
    /* Test for correct number of arguments */
    memset(&echoServAddr, 0, sizeof(echoServAddr)); /* Zero out structure */
    if ((argc < 3) || (argc > 4))    /* Test for correct number of arguments */
    {
       fprintf(stderr, "Usage: %s <Server Hostname/IP> <Echo Word> [<Server Port/Service>]\n",
               argv[0]);
       exit(1);
    }
    servIP = argv[1];             /* First arg: server IP address (dotted quad) */
    echoString = argv[2];         /* Second arg: string to echo */
    if (argc == 4){
        char numbers[] = "0123456789";//check for an int
        for(int i=0;i<10;i++){
            if((argv[3])[0] == (int)numbers[i]){
                printf("Found a User Defined Port\n");
                echoServAddr.sin6_port = htons(atoi(argv[3])); /* Use given port, if any */
                userport = 1;
            }
        }
        if(userport==0){
            printf("Found a Service Searching For The Port Now...\n");
            //we search the services file for entries matching the name and with the type sctp
            struct servent *sp;//the structure pointer for the dns search
            sp = getservbyname(argv[3],"sctp");//only checking dns for sctp
            if (sp == NULL) {
                fprintf(stderr, "Service Failed To Find The Port For sctp\nmake sure that your services file contains the hostname with the port/sctp\n");
                exit(1);
            }
            //the service was found add it to structure in binary format
            echoServAddr.sin6_port = (sp->s_port);//assign the return of the structure as its already in binary format
            printf("Found The Port Of %s [%hu]\n",argv[3],ntohs(sp->s_port));
        }
    }else{
        echoServAddr.sin6_port = htons(7); //if the user dosent give a port we assign it at 7
    }
    /* ------Step 1 create socket --------------- */
    if ((sock_Descr = socket(PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP)) < 0)
        DieWithError("socket() failed");
    if (setsockopt(sock_Descr, IPPROTO_SCTP, SCTP_EVENTS, &sctpEventSub, sizeof(sctpEventSub)) < 0 )
        DieWithError("setsocketopt() SCTP_Events failed");
//---------------------------------------------------------------------------
    struct addrinfo hints, *infoptr;
    hints.ai_family = AF_INET6; //only searching dns for ipv6 addresses
    //first we check if the address the user entered is an ipv6 address
    //if so we just copy it into the structure
    echoServAddr.sin6_family = AF_INET6;
    echoServAddr.sin6_len = sizeof(struct sockaddr_in6);
    echoStringLen = strlen(echoString); /* Determine input length */
    if (inet_pton(AF_INET6, servIP, &echoServAddr.sin6_addr) > 0) {
        printf("Found a user Defined IPV6 Address\n");
        //note inet_pton will take care of setting the address
    }else{//if the address isnt ipv6 we look for it in the dns
        printf("Found a Hostname Searching For The Address Now...\n");
        int result = getaddrinfo(servIP, NULL, &hints, &infoptr);
        if (result) {
            fprintf(stderr, "Failed to find the ipv6 hostname make sure you inputed the address or hostname correctly %s\n", gai_strerror(result));
            exit(1);
        }
        char host[INET6_ADDRSTRLEN];//create the string to hold the address
        getnameinfo(infoptr->ai_addr, infoptr->ai_addrlen, host, sizeof (host), NULL, 0, NI_NUMERICHOST);//serch dns for entry matching name
        inet_pton(AF_INET6, host, &echoServAddr.sin6_addr);//copy the address found into the structure
        sprintf(hostipaddr,"%s",host);
        printf("Found The IPV6 Address Of %s [%s]\n",servIP,host);
    }
    inet_ntop(AF_INET6, &echoServAddr.sin6_addr, hostipaddr,
              sizeof(hostipaddr));
    
    printf("Connecting To SCTP Server [%s:%hu]\n",hostipaddr,ntohs(echoServAddr.sin6_port));
    /* ------Step 2 send message to server ------- */
    /* Send the string to the server */
//------------------------------------
// Lab 7 Part B - comment this section - what are the fields for?
     ret = connect (sock_Descr, (struct sockaddr *) &echoServAddr, sizeof (echoServAddr));
    servLen = sizeof(echoServAddr);
    bzero(&sndrInfo, sizeof(sndrInfo));
    if ((sendSize = sctp_sendmsg(sock_Descr,
                    echoString,
                    echoStringLen,
                    (struct sockaddr *) &echoServAddr,
                    servLen,
                    0, 0, 
                    sndrInfo.sinfo_stream,
                    0,0            )) != echoStringLen)
        DieWithError("send() sent a different number of bytes than expected");
        printf("Sent to the Server: [%.*s]\n",echoStringLen, echoString);      /* Print the echo buffer */
//------------------------------------
        
    /* ------Step 3 recv message from server ------ */
    /* Receive the same string back from the server */
//------------------------------------
// Lab 7 Part B - comment this section - what are the fields for?
    if ((totalBytesRcvd = sctp_recvmsg(sock_Descr,
                          echoString,
                          sizeof(echoStringLen),
                          (struct sockaddr *) &echoServAddr,
                          &servLen,
                          &sndrInfo,
                          &msgFlags)) < 0 )
        DieWithError("recv() had an error ");
    else {
        print_src(sock_Descr, sndrInfo.sinfo_assoc_id);
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
static void print_src(int fd, sctp_assoc_t assoc_id)
{
// Lab 7 Part B - comment this section - What are the fields for?
        struct sctp_status sstat;
        struct sctp_paddrinfo *spinfo;
        char tmpname[INET_ADDRSTRLEN];
        unsigned int port;
        unsigned int ulen;
        struct sockaddr_in6 *s_in;

        bzero(&sstat, sizeof (sstat));

        ulen = sizeof (sstat);
// Lab 7 Part B - comment this section - What values are we interested in returning?
        if (sctp_opt_info(fd, assoc_id, SCTP_STATUS, &sstat, &ulen) < 0) {
                perror("sctp_opt_info()");
                return;
        }
        spinfo = &sstat.sstat_primary;

// Lab 7 Part B - comment this section - What are we doing on each line?
        s_in = (struct sockaddr_in6 *) &spinfo->spinfo_address;
        inet_ntop(AF_INET6, &s_in->sin6_addr, tmpname, sizeof (tmpname));
        port = ntohs(s_in->sin6_port);
        printf("Msg from Server on association - %d IP:Port - %s:%d\n", assoc_id, tmpname, port);
}
//------------------------------------

