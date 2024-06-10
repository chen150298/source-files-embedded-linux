CC=/home/chen/Documents/buildroot/buildroot-2024.02/output/host/bin/arm-linux-gcc

default: client server
	
client:
	gcc client.c -o client

server:
	$(CC) server.c -o server
	
clean:
	rm client server
