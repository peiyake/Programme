CC=gcc
OBJ1=broadcastsrv
OBJ2=broadcastcli
all:
	$(CC) $(OBJ1).c -DSERVER -o $(OBJ1)
	$(CC) $(OBJ2).c -o $(OBJ2)

clean:
	rm -f $(OBJ1) $(OBJ2)
