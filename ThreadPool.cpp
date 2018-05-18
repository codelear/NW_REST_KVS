#include <queue>
#include <string>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <condition_variable>

using namespace std;

class ThreadPool
{
	vector<thread> threadlist;
	queue<pair<string, int> > requestQueue;
	mutex qLock;
	condition_variable queued;
	volatile bool breakout;

	void (*callback)(string,int);

public:

	ThreadPool()
	{
		breakout=false;
	}

	void setcallback(void (*fp)(string, int))
	{
		callback=fp;	
	}

	void queuerequest(string request, int s)
	{
		{
			unique_lock<mutex> lock(qLock);
			requestQueue.push(make_pair(request,s));	
		}	
		queued.notify_one();	
	}	

	void worker()
	{
		while (!breakout)
		{
			pair<string, int> request;
			{
				unique_lock<mutex> lock(qLock);

				while((!breakout) && (requestQueue.empty()))
				{
					queued.wait(lock);		
				}	

				if (!requestQueue.empty())
				{
					request = requestQueue.front();
					requestQueue.pop();
				}
			}

			if (!request.first.empty())
			{
				callback(request.first,request.second);
			}
		}

	}

	void createthreads(int count)
	{
		for (int i=0;i<count;i++)
		{
			threadlist.push_back(thread(&ThreadPool::worker,this));	
		}
	}	

	void endthreads()
	{
		breakout=true;
		queued.notify_all();
		for (int i=0;i<threadlist.size();i++)
		{
			threadlist[i].join();
		}
	}

};

