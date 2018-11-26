OBJS	= Server.o,CaesarCipher.o,Application.o,Client.o
OUT	= Server,Client

OBJS0	= Server.o CaesarCipher.o
SOURCE0	= Server.c CaesarCipher.c
HEADER0	= 
OUT0	= Server

OBJS1	= Application.o Client.o CaesarCipher.o
SOURCE1	= Application.c Client.c CaesarCipher.c
HEADER1	= 
OUT1	= Client

CC	 = gcc
FLAGS	 = -g -c -Wall
LFLAGS	 = 

all: Server Client

Server: $(OBJS0) $(LFLAGS)
	$(CC) -g $(OBJS0) -o $(OUT0)

Client: $(OBJS1) $(LFLAGS)
	$(CC) -g $(OBJS1) -o $(OUT1)

Server.o: Server.c
	$(CC) $(FLAGS) Server.c 

CaesarCipher.o: CaesarCipher.c
	$(CC) $(FLAGS) CaesarCipher.c 

Application.o: Application.c
	$(CC) $(FLAGS) Application.c 

Client.o: Client.c
	$(CC) $(FLAGS) Client.c 


clean:
	rm -f $(OBJS) $(OUT)