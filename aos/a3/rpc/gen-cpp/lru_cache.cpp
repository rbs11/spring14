/*
 * lru_cache.cpp
 *
 *  Created on: Mar 26, 2014
 *      Author: root
 */
#include "webproxy.h"
#include "cache.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <sys/select.h>
#include <curl/curl.h>
#include <stdio.h>
#include <iostream>

using namespace  ::Rpc;

using namespace std;
struct hash_value{
	uint32_t lru_cnt;
	webpage cached_pg;
};
typedef std::map<std::string, hash_value> url_map;
typedef std::map<uint32_t, std::string> lru_map;
#define  DEBUG
//#define DEBUG_EXTRA
url_map web_cache;
lru_map lru_table;

int32_t cache_sz_total;
int32_t cache_sz_remaining;

uint64_t glru_count;
//class cache{
void set_cache_size(uint32_t size){
	cache_sz_total = cache_sz_remaining = size;

}
void add_to_cache(std::string url, webpage& current_page){

	hash_value pg_data;
	lru_map::iterator itr;

	if (cache_sz_total < current_page.webpage_len)
	{
		//error handle. page too big for cache
		cout<<"Cache too small for url "<<url;
		return;
	}

	/*Evict lru elements */
	while (cache_sz_remaining < current_page.webpage_len)
	{
		itr = lru_table.begin();
		cache_sz_remaining += web_cache[itr->second].cached_pg.webpage_len;
#ifdef DEBUG
		cout<<"\tEvicting URL "<<itr->second<<"\n";
#endif
		web_cache.erase(itr->second);
		lru_table.erase(itr);
	}

	/*Add to cache*/
	pg_data.cached_pg = current_page;
	pg_data.lru_cnt = glru_count++;

	cache_sz_remaining -= current_page.webpage_len;
#ifdef DEBUG_EXTRA
	cout<<"Caching Page data for Url "<<url<<" Length "<<current_page.webpage_len<<":\n"<< current_page.webpage_data<<"\n";
	cout<<"Cache size remaining after new addition"<<cache_sz_remaining<<"\n";
#endif
	web_cache[url] = pg_data;
	lru_table[pg_data.lru_cnt] = url;
	return;
}

int32_t get_cached_page(std::string url, webpage& returned_page)
{
	uint32_t prev_lru, curr_lru;

	if (web_cache.find(url) == web_cache.end())
	{
		cout<<" URL "<<url<<" not found in cache \n";
		return -1;
	}
	prev_lru = web_cache[url].lru_cnt;
	lru_table.erase(prev_lru);

	curr_lru = glru_count;
	lru_table[curr_lru] = url;

	web_cache[url].lru_cnt = glru_count++; /*Update LRU for just accessed page*/
	returned_page = web_cache[url].cached_pg;

#ifdef DEBUG_EXTRA
	cout<<"Cached Page data for Url "<<url<<" Length "<<returned_page.webpage_len<<":\n"<< returned_page.webpage_data<<"\n";
#endif
	return 0;
}

//};
