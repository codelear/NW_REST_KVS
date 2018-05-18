Server:	SocketServer.o KV_Store.o KV_Persist.o ThreadPool.o
	g++ -ggdb -o Server SocketServer.o KV_Store.o KV_Persist.o ThreadPool.o -L. -I. -pthread -std=gnu++0x

SocketServer.o:	SocketServer.cpp SocketServer.h KV_Persist.o KV_Store.o
	g++ -c -ggdb SocketServer.cpp -o SocketServer.o -L. -I. -std=gnu++0x

ThreadPool.o:	ThreadPool.cpp
	g++ -c -ggdb ThreadPool.cpp -o ThreadPool.o -L. -I. -std=gnu++0x -pthread

KV_Store.o:	KV_Store.cpp KV_Persist.o 
	g++ -c -ggdb KV_Store.cpp -o KV_Store.o -I. -L. -std=gnu++0x

KV_Persist.o:	KV_Persist.cpp
	g++ -c -ggdb KV_Persist.cpp -o KV_Persist.o -I. -L. -std=gnu++0x 
