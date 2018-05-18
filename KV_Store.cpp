#include "KV_Persist.cpp"
#include <iostream>
#include <map>
#include <list>
#include <utility>

using namespace std;

class KV_Store
{
	/*
	* *list store pair because when LRU entry is removed from cache
	* * we need the corresponding key to remove the entry for the index
  */
	list<pair<string, string> > cache;
	map<string, list<pair<string, string> >::iterator > index;
	long int cacheSizeBytes;
	long int currSizeBytes;

	PersistentMap serializer; 

public:
	KV_Store(){}
	KV_Store(long int bytes)
	{
		setCacheSize(bytes);
		currSizeBytes=0;	
	}

	void setCacheSize(long int bytes)
	{
		cacheSizeBytes = bytes;
	}

	string get(string key)
	{
		map<string, list<pair<string, string> >::iterator >::iterator index_entry;
		index_entry = index.find(key);
		string value;

		/*
		* *if entry exists in the index
		* *get the pointer to the cache
		* *get the entry from the cache
		* *delete the entry from the cache
		* *insert the same entry to the start of cache
		* *update the index to make the key point to the new location of the data in the cache i.e. top of cache
    */

		if (index_entry != index.end())
		{
			list<pair<string, string> >::iterator cache_entry;
			cache_entry = index_entry->second;
			pair<string, string> data = *cache_entry;
			cache.erase(cache_entry);
			cache.push_front(data);
			index[key]=cache.begin();
			value=data.second;
		}
		/*
		* *if it is not in the index
		* *check if it is in the file and bring it in
		* *otherwise throw exception
		* *or return null
		* *insert the entry to the top of the list
		* *insert key in the index to point to the entry in the list
		*/
		else
		{
			bool found = serializer.retrieve(key, value);
			if (found)
			{	
				put(key, value);
			}

		}

		return value;   
	}

	void put(string key, string value)
	{
		long int size = key.length()+value.length();

		/*
		* *check if key is present then update else insert
		* *free up space to ensure the key and value to be inserted 
		* *have space in the cache
		* *then when sufficient space is available insert in cache and index
		* *update the cache size
		*/ 

		map<string, list<pair<string, string> >::iterator >::iterator index_entry;
		index_entry = index.find(key);

		if (index_entry!=index.end())
		{
			list<pair<string, string> >::iterator cache_entry = index_entry->second;
			pair<string, string> data = *cache_entry;
			cache.erase(cache_entry);
			int oldvaluelen = data.second.length();
			data.second=value;
			currSizeBytes = currSizeBytes - (key.length() + oldvaluelen);

			while ((currSizeBytes+size > cacheSizeBytes) && (currSizeBytes != 0))
			{
				pair<string, string> last_entry = cache.back();
				deletekey(last_entry.first, true);
			}	

			cache.push_front(data);
			index[key]=cache.begin();
			currSizeBytes = currSizeBytes + size;

		}
		else
		{
			while ((currSizeBytes+size > cacheSizeBytes) && (currSizeBytes != 0))
			{
				pair<string, string> last_entry = cache.back();
				deletekey(last_entry.first, true);
			}
			pair<string, string> cache_data = make_pair(key,value);
			cache.push_front(cache_data);
			list<pair<string, string> >::iterator cache_front = cache.begin();
			pair<string, list<pair<string, string> >::iterator> index_entry = make_pair(key,cache_front);
			index.insert(index_entry);
			currSizeBytes = currSizeBytes + size;
		}
	}


	/*
	* *delete from the cache
	* *delete correponding entry in index
	* *persist to disk
	* *update cache size
	*/
	void deletekey(string key,bool persist=false)
	{
		map<string, list<pair<string, string> >::iterator >::iterator index_entry;
		index_entry = index.find(key);
		if (index_entry != index.end())
		{
			list<pair<string, string> >::iterator cache_entry;
			cache_entry=index_entry->second;
			string value=cache_entry->second;
			long int value_len=value.length();
			cache.erase(cache_entry);
			index.erase(index_entry);

			/*persist*/
			if (persist)
			{
				serializer.persist(key,value);
			}
			else
			{
				serializer.remove(key);
			}

			currSizeBytes = currSizeBytes -  (key.length() + value_len);
		}
		/*
		* *maybe key is present in disk 
		* *then remove from disk
		*/
		else
		{
			serializer.remove(key);	
		}
	}

	void displaylist()
	{
		cout << endl;
		for (list<pair<string, string> >::iterator i = cache.begin(); i != cache.end(); ++i)
			cout << " Key = " <<(*i).first <<" Value = " << (*i).second << " In Index = " << inCache((*i).first)<<endl;
	}

	bool inCache(string key)
	{
		map<string, list<pair<string, string> >::iterator >::iterator index_entry;
		index_entry = index.find(key);
		if (index_entry != index.end())
		{
			list<pair<string, string> >::iterator cache_entry;
			cache_entry = index_entry->second;
			pair<string, string> data = *cache_entry;			

			//cout <<"\t Index entry = " << data.second; 
			return true;
		}
		else
		{
			return false;
		}

	}
};

