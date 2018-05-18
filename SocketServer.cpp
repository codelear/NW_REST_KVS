#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string>
#include <string.h>
#include <bits/stdc++.h>
#include <csignal>
#include <mutex>
#define PORT 6080
#include "SocketServer.h"
#include "KV_Store.cpp"
#include "ThreadPool.cpp"

KV_Store cache(10);
volatile bool done;
int server_fd;
mutex cacheLock;
ThreadPool threadpool;

void SIGHandler(int signal)
{
	close(server_fd);
	done=1;
}

void GETHandler(int socket, string key)
{
	std::string response;
	cacheLock.lock();
	string result=cache.get(key);	
	cacheLock.unlock();
	response=GET_RESPONSE_JSON+"\r\n{\"key\":\""+key+"\",\"value\":\""+result+"\"}";
	send(socket , response.c_str() , response.length() , 0 );
	close(socket);
}

void PUTHandler(int socket, string key, string value)
{
	cacheLock.lock();
	cache.put(key, value);
	cacheLock.unlock();
	std::string response;
	response=GET_RESPONSE_JSON+"\r\n{\"result\":\"success\"}";
	send(socket , response.c_str() , response.length() , 0 );
	close(socket);
}

void DELETEHandler(int socket, string key)
{
	std::string response;
	cacheLock.lock();	
	cache.deletekey(key);
	cacheLock.unlock();
	response=GET_RESPONSE_JSON+"\r\n{\"result\":\"success\"}";
	send(socket , response.c_str() , response.length() , 0 );
	close(socket);
}

void workerHandler(string buffer, int new_socket)
{
	if (buffer.compare(0,3,GET_VERB)==0)
	{
		int keyendpos = buffer.find("HTTP\0",4);
		std::string key(buffer, 5, keyendpos-1-5);
		GETHandler(new_socket, key);
	}
	else if (buffer.compare(0,3,PUT_VERB)==0)
	{
		string request(buffer);
		int value_start = request.rfind("\"value\":");
		int value_end = request.rfind("\"}");
		string value(request,value_start+9,value_end-(value_start+9));
		int key_start =  request.rfind("\"key\":");
		string key(request,key_start+7,value_start-(key_start+7)-2);
		PUTHandler(new_socket, key, value); 	
	}
	else if (buffer.compare(0,6,DELETE_VERB)==0)
	{
		int keyendpos = buffer.find("HTTP\0",7);
		std::string key(buffer,8,keyendpos-1-8);
		DELETEHandler(new_socket, key);
	}
}

void processrequest(int new_socket)
{
	char buffer[1024] = {0};
	int valread = read( new_socket , buffer, 1024);
	std::string request(buffer);
	threadpool.queuerequest(request, new_socket);
}

int main(int argc, char const *argv[])
{
	if (argc==2)
	{
		stringstream ss(argv[1]);
		int size;
		ss >> size;
		if (!ss.fail())
			cache.setCacheSize(size);		
		std::cout << size;
	}
  
	int  new_socket;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( PORT );

	if (::bind(server_fd, (struct sockaddr *)&address,sizeof(address))<0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	if (listen(server_fd, 3) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	signal(SIGINT, SIGHandler);

	threadpool.setcallback(workerHandler);
	threadpool.createthreads(5);

	while (!done)
	{	
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))>0)
		{
			processrequest(new_socket);
		}
	}
	threadpool.endthreads();
}
