/*
 * cache.h
 *
 *  Created on: Mar 26, 2014
 *      Author: root
 */

#ifndef CACHE_H_
#define CACHE_H_
using namespace ::Rpc;

//class cache{
void set_cache_size(uint32_t size);
void add_to_cache(std::string url, webpage& current_page);
int32_t get_cached_page(std::string url, webpage& returned_page);
//};
#endif /* CACHE_H_ */
