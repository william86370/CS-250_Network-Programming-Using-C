
// TCP Client program

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
#define PORT 5000
#define MAXLINE 1024
int main()
{
    char buffer[]="IDCC302672";
    if (strncmp(buffer, "IDCC", 4) == 0) {
        char countryid[MAXLINE] ;
        sscanf(buffer, "IDCC%s", &countryid);
        char line[256];
        bzero(line, sizeof(line));
        char filename[] = "Country_DB.csv";
        FILE *file = fopen(filename, "r");
        if ( file != NULL )
        {
            while (fgets(line, sizeof line, file) != NULL) /* read a line */
            {
                if (strncmp(line,countryid , 6) == 0) {
                    
                    int cid=0;
                    char cname[80];
                    char countrycode[3];
                    char ccon[3];
                    sscanf(line,"%d,%2[^,],%80[^,],%2[^,]",&cid,countrycode,cname,ccon);
                    //printf("%d,%s,%s,%s\n",cid,countrycode,cname,ccon);
                    printf("%s\n",ccon);
                }
            }
            fclose(file);
        }
        else
        {
            //file doesn't exist
            
            exit(1);
        }
    }
}
