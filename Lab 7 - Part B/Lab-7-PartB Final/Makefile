CC = gcc
CFLAGS = -g -Wall
SCTPFLAG = -lsctp
INCLUDE1=DieWithError.c

all: EchoClientMto1SCTP EchoServerMto1SCTP

EchoClientMto1SCTP: EchoClientMto1SCTP.c
	$(CC) $(CFLAGS) $@.c $(INCLUDE1) -o $@.o $(SCTPFLAG)

EchoServerMto1SCTP: EchoServerMto1SCTP.c
	$(CC) $(CFLAGS) $@.c $(INCLUDE1) -o $@.o $(SCTPFLAG)

clean:
	-rm -f *.o *~ *core*
