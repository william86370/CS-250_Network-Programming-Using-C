//
//  Application.c
//  CS-250-Project-1
//
//  Created by william wright on 11/15/18.
//  Copyright © 2018 Silvertap. All rights reserved.
//
#include <stdio.h>//default libarry
#include <stdlib.h>//default libarry
#define MAXLINE 1024//the maxsize of our buffer
//we use connectionhelper.c but xcode dosent make me include it

void menu(int choicein,int PORT,char address[]);//the function that holds the menu selection
void Setup(int config[1],char address[]);//the function handles the setup of the program and checks for settings

int main(int argc, const char * argv[]) {
    //main runs inside of a while loop to only exit on failure or when the user selects to do so
    int num;//the selection that the user picks
    //-----------------------------------------------------------------------------------------------|
    //Setup Of the Server Here We check for the files and Also Load/Create a Config For Settings     |
    //-----------------------------------------------------------------------------------------------|
    int PORT[1];//the Port we will use
    char address[MAXLINE];
     int status;
    Setup(PORT,address);//the function that handles setup of the server

    printf("|------------------------------------------------------------------------------------------|\n");
    printf("|                              CS-250 Project-1 UDP-TCP CLient                             |\n");
    printf("|                                Developed By William Wright                               |\n");
    printf("|               This Client Will Have A menu For the user to select actions on             |\n");
    printf("|      The User Will Beable to Request the time messages of the day and do county lookup   |\n");
    printf("|------------------------------------------------------------------------------------------|\n");
   
    do{
        printf("|-----------------------------------------------------|\n");
        printf("|                      Main Menu                      |\n");
        printf("|     Please Pick A Number From The Options Below     |\n");
        printf("|                                                     |\n");
        printf("|     0.) Select To Exit The Program                  |\n");
        printf("|     1.) Request Date And Time From The Server       |\n");
        printf("|     2.) Request Quote Of the day From The Server    |\n");
        printf("|     3.) Request Country Lookup based upon ID        |\n");
        printf("|     4.) Reset Client Config File                    |\n");
        printf("|-----------------------------------------------------|\n");
        printf("Option: ");
         scanf("%d", &num);
        if(num==1||num==2||num==3){
             menu(num,PORT[0],address); //call to our super function for hanleing user input
        }else if(num==4){
            status = remove("ClientConfig.ini");
            if (status == 0){
                printf("removed Client Settings\n");
                Setup(PORT,address);//the function that handles setup of the server4
            }else{
                exit(1);
            }
        }else{
             printf("Error You Didnt Enter An Option From The List\n");
        }
    }while(num !=0);
    return 0;
}

void menu(int choicein,int PORT,char address[]){
    int numchoice;
    if(choicein==1){//date and time
        do{
            printf("|-----------------------------------------------------|\n");
            printf("|                      Time Menu                      |\n");
            printf("|     Please Pick A Number From The Options Below     |\n");
            printf("|                                                     |\n");
            printf("|     0.) Select To Return To The Main Menu           |\n");
            printf("|     1.) Request Date And Time In GMT Format         |\n");
            printf("|     2.) Request Date And Time In Local Format       |\n");
            printf("|-----------------------------------------------------|\n");
            printf("Option: ");
        scanf("%d", &numchoice);
            if(numchoice==1||numchoice==2||numchoice==0){
            if(numchoice==1){
                gettime(1,PORT,address);
                return;
            }else if(numchoice==2){
                gettime(2,PORT,address);
                return;
            }
            }else{
                printf("\nPlease Enter A Valid Choice");
            }
        }while(numchoice !=0);
    }
    if(choicein==2){
        getqotd(PORT,address);
        return;
    }
    if(choicein==3){
        do{
            printf("|-----------------------------------------------------|\n");
            printf("|                Country Selecrion Menu               |\n");
            printf("|     Please Pick A Number From The Options Below     |\n");
            printf("|                                                     |\n");
            printf("|     0.) Select To Return To The Main Menu           |\n");
            printf("|     1.) Select To Enter Id to get Country Code      |\n");
            printf("|     2.) Select To Enter Id to get Country Name      |\n");
            printf("|     3.) Continent the Country is associated with    |\n");
            printf("|-----------------------------------------------------|\n");
            printf("Option: ");
            scanf("%d", &numchoice);
            if(numchoice==1||numchoice==2||numchoice==0||numchoice==3){
                if(numchoice==1){
                    printf("Please Enter The Code For the country you wish to lookup: ");
                    int code;
                    scanf("%d", &code);
                    Countryselectorhelper(numchoice,code,PORT,address);
                    break;
                }else if(numchoice==2){
                    printf("Please Enter The Code For the country you wish to lookup: ");
                    int code;
                    scanf("%d", &code);
                    Countryselectorhelper(numchoice,code,PORT,address);
                    break;
                }else if(numchoice==3){
                    printf("Please Enter The Code For the country you wish to lookup: ");
                    int code;
                    scanf("%d", &code);
                   Countryselectorhelper(numchoice,code,PORT,address);
                    break;
                }
            }else{
                printf("\nPlease Enter A Valid Choice");
            }
        }while(numchoice !=0);
    }
        return;
}
void Setup(int config[1],char address[]){
    //the setup allows us to check for the required files prior to starting the Client
    //we check for the config file to get Port and server address
    FILE *fp;//main file header
    //--------------------------------------------|
    //The Config file that the Client uses        |
     char filename3[] = "ClientConfig.ini";// |
    //--------------------------------------------|
    
    fp = fopen(filename3, "r");
    if (fp == NULL)
    {
        //if the file dosent exit then we create it
        fclose(fp);
        printf("The Config File Couldnt be found begining First Time Setup\n");
        fp = fopen(filename3, "w+");
        int po;
        char add[20];
        printf("Enter The Port The Server Should Use For Connection\nPort: ");
        scanf("%d",&po);
        printf("Enter The IP Address Of the Server\nIP: ");
        scanf("%s",add);
        fprintf(fp,"%d %s",po,add);
        printf("Config File Created\n");
        config[0]=po;
        memset(address, 0, sizeof(MAXLINE));
        sprintf(address,"%s",add);
        fclose(fp);
         printf("Loaded Server Config [Port:%d IP Address:%s]\n",config[0],address);
    }else{
        //the config file exists
        printf("Found Server Config File, Loading Values...\n");
        int po;
         char add[20];
        fscanf(fp,"%d %s",&po,add);
        if(po==0){
            config[0]=5000;//default value
            sprintf(address,"0.0.0.0");
            printf("The Config contained an Error, Setting Values to Default [Port:%d,IP Address:%s]\n",config[0],address);
            fclose(fp);
            fp = fopen(filename3, "w+");
            fprintf(fp,"%d %s",config[0],address);
        }else{
            memset(address, 0, sizeof(MAXLINE));
            sprintf(address,"%s",add);
            config[0]=5000;//default value
            printf("Loaded Server Config [Port:%d IP Address:%s]\n",config[0],address);
        }
        fclose(fp);
    }
    printf("Setup Complete All Files Ready...\n");
    return;
}

