#include <iostream>
#include <fstream>
#include <string> 
#include <sstream>
#include <vector>
#include <cstdlib>
#include <map>
#include <utility>
#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>

using namespace std;

class PersistentMap
{
	map<string, string> cache;
	thread writer;
	mutex lockcache;
	volatile bool isexit;

	void readin()
	{
		ifstream inputfile;
		inputfile.open("KV.cache");

		if (inputfile.is_open())
		{
			string line;
			//while(inputfile>>line)
			while(getline(inputfile,line))
			{
				stringstream ss(line);
				string key;
				string value;
				string delim;

				while(ss>>key)
				{
					ss>>value;
					lockcache.lock();					
					cache.insert(make_pair(key,value));
					lockcache.unlock();
				}
			}
			inputfile.close();	
		}
	}

	void writercallback()
	{
		while(!isexit)
		{
			writeout();			
			sleep(60);	
		}	
	}

public:

	PersistentMap()
	{
		readin();
		isexit=false;
		writer = thread(&PersistentMap::writercallback, this);
	}

	void writeout()
	{
		lockcache.lock();
		ofstream outfile;
		outfile.open("KV.cache");

		for(map<string, string>::iterator it=cache.begin(); it!=cache.end(); ++it)
		{
			outfile <<it->first<<"\t"<<it->second<<"\n";
		}
		outfile.close();
		lockcache.unlock();
	}


	void persist(string key, string value)
	{
		pair<string,string> data=make_pair(key,value);
		lockcache.lock();
		cache[key]=value;
		lockcache.unlock();
	}

	void remove(string key)
	{
		lockcache.lock();
		cache.erase(key);
		lockcache.unlock();
	}

	~PersistentMap()
	{
		isexit=true;
		writer.join();
		writeout();
	}

	void displaymap()
	{
		lockcache.lock();
		for(map<string, string>::iterator it=cache.begin(); it!=cache.end(); ++it)
		{
			cout <<"<"<<it->first<<", "<<it->second<<">\n";
		}
		lockcache.unlock();

	}

	bool retrieve(const string key, string &value)
	{
		lockcache.lock();
		map<string, string>::iterator entry = cache.find(key);
		if (entry!=cache.end())
		{
			value=entry->second;
			lockcache.unlock();
			return true;
		}
		lockcache.unlock();
		return false;
	}

};




