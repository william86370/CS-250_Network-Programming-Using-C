CC = gcc
CFLAGS = -g -Wall
SCTPFLAG = -lsctp
INCLUDE1=DieWithError.c

all: EchoClientSCTPMto1v6 EchoServerSCTPMto1v6

EchoClientSCTPMto1v6: EchoClientSCTPMto1v6.c
	$(CC) $(CFLAGS) $@.c $(INCLUDE1) -o $@.o $(SCTPFLAG)

EchoServerSCTPMto1v6: EchoServerSCTPMto1v6.c
	$(CC) $(CFLAGS) $@.c $(INCLUDE1) -o $@.o $(SCTPFLAG)

clean:
	-rm -f *.o *~ *core*
