#fisier folosit pentru compilarea serverului&clientului TCP iterativ

all:
	g++ serverconcurent.cpp -lsqlite3 -o server.exe
	g++ clientTCP.cpp -o client.exe
clean:
	rm -f *~cliTcpIt concurent.exe
