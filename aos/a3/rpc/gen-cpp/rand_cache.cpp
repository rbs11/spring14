/*
 * rand_cache.cpp
 *
 *  Created on: Mar 26, 2014
 *      Author: root
 */

/*
 * lru_cache.cpp
 *
 *  Created on: Mar 26, 2014
 *      Author: root
 */
#include "webproxy.h"
#include "cache.h"
#include <map>
#include <iterator>
#include <stdio.h>

using namespace  ::Rpc;

using namespace std;
struct hash_value{
	uint32_t lru_cnt;
	webpage cached_pg;
};
typedef std::map<std::string, hash_value> url_map;
#define  DEBUG
//#define DEBUG_EXTRA
url_map web_cache;

int32_t cache_sz_total;
int32_t cache_sz_remaining;

void set_cache_size(uint32_t size){
	cache_sz_total = cache_sz_remaining = size;

}
void add_to_cache(std::string url, webpage& current_page){

	hash_value pg_data;
	url_map::iterator itr;

	if (cache_sz_total < current_page.webpage_len)
	{
		//error handle. page too big for cache
		cout<<"Cache too small for url "<<url;
		return;
	}

	/*Evict lru elements */
	while (cache_sz_remaining < current_page.webpage_len)
	{
		itr = web_cache.begin();
		std::advance( itr, rand()%(web_cache.size()) );
		cache_sz_remaining += itr->second.cached_pg.webpage_len;
#ifdef DEBUG
		cout<<"\tEvicting URL "<<itr->first<<"\n";
#endif
		web_cache.erase(itr->first);
	}

	/*Add to cache*/
	pg_data.cached_pg = current_page;

	cache_sz_remaining -= current_page.webpage_len;
#ifdef DEBUG_EXTRA
	cout<<"Caching Page data for Url "<<url<<" Length "<<current_page.webpage_len<<":\n"<< current_page.webpage_data<<"\n";
	cout<<"Cache size remaining after new addition"<<cache_sz_remaining<<"\n";
#endif
	web_cache[url] = pg_data;
	return;
}

int32_t get_cached_page(std::string url, webpage& returned_page){

	if (web_cache.find(url) == web_cache.end())
	{
		cout<<" URL "<<url<<" not found in cache \n";
		return -1;
	}
	returned_page = web_cache[url].cached_pg;

#ifdef DEBUG_EXTRA
	cout<<"Cached Page data for Url "<<url<<" Length "<<returned_page.webpage_len<<":\n"<< returned_page.webpage_data<<"\n";
#endif
	return 0;
}

//};
