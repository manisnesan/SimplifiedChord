
default: client
	gcc -o dhtmain dhtmain.c -lcrypto -lm

client:
	gcc -o client client.c 
	
clean:
	rm dhtmain client nodelist
	
