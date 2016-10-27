all: client server

client: ../client_src/myftp.c
	gcc ../client_src/myftp.c -o ../client_src/client -lmhash

server: ../server_src/tcpserver.c
	gcc ../server_src/tcpserver.c -o ../server_src/server -lmhash

clean:
	rm ../server_src/server
	rm ../client_src/client
