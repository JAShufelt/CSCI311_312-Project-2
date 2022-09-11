all: Server.out Interface.out

Server.out: Server.c Server.h gasData
	gcc Server.c -o Server.out

Interface.out: Interface.c Interface.h
	gcc Interface.c -o Interface.out
