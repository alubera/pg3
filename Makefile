all: client server

client: myftp.c
	gcc myftp.c -o client -lmhash

server: tcpserver.c
	gcc tcpserver.c -o server -lmhash

clean:
	rm server
	rm client
