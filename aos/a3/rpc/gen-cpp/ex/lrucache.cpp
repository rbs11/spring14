#include "web_cache.h"

static int max_size;
static int cache_size;

static int32_t time_stamp = 0;

/* Use STL Map container to store url<->[webpage, time_stamp_to_url] mapping */
typedef std::map<std::string, std::pair<webpage, int32_t> > uw_map;
static uw_map url_to_webpage;

/* Use STL multi-map container to store time_stamp_to_url<->url mapping for eviction */
typedef std::map<int32_t, std::string> tu_map;
static tu_map time_stamp_to_url;

class web_cache{

public:
/* initialize webcache */
void init_web_cache(const int size)
{
    max_size = size;
    cache_size = 0;
}

/* is webpage in cache? */
bool check_web_cache(const std::string& url)
{
    if (max_size == 0) /* 0 byte cache OR no-cache */
        return false;

    if (url_to_webpage.size() == 0) /* webcache is empty */
        return false;

    if (url_to_webpage.count(url) != 0) /* in webcache */
        return true;
    else /* not in webcache */
        return false;
}

/* returns the webpage from cache */
void get_webpage_from_cache(webpage& _return,
        const std::string& url)
{
    /*updating the time_stamp*/
    time_stamp++;

    std::cout<<"Getting page "<<url<<" from cache \n";
    /* safety check */
    assert(url_to_webpage.size() != 0);

    uw_map::iterator it1;

    /* get webpage from cache */
    it1 = url_to_webpage.find(url);
    assert(it1 != url_to_webpage.end());

    _return.webpage_data.assign(it1->second.first.webpage_data);
    _return.webpage_len = it1->second.first.webpage_len; 

    int32_t ts = it1->second.second;

    /* adjust time_stamp of url in url_to_webpage map */
    it1->second.second = time_stamp;

    /* adjust time_stamp in time_stamp_to_url multimap */
    //std::pair <tu_map::iterator, tu_map::iterator> ret;

    tu_map::iterator it2;
    it2 = time_stamp_to_url.find(ts);
    
    time_stamp_to_url.erase(it2);

    time_stamp_to_url.insert(std::pair<int32_t, std::string>((time_stamp),
                                url));
    
}

static void evict_lru_page(void)
{
    /* safety check */
    assert(time_stamp_to_url.size() != 0);
    assert(url_to_webpage.size() != 0);

    /* get LFU url */
    tu_map::iterator it1 = time_stamp_to_url.begin();
    std::string url;
    url.assign(it1->second);

   std::cout<<"Evicting page "<<url<<" from cache \n";

    /* get the webpage iterator */
    uw_map::iterator it2 = url_to_webpage.find(url);

    /* decrement cache size */
    cache_size -= it2->second.first.webpage_len;

    /* erase from both maps */
    url_to_webpage.erase(it2);
    time_stamp_to_url.erase(it1);
}

/* store the webpage in cache */
void put_webpage_to_cache(const webpage& _input,
        const std::string& url)
{
    /* If web page is bigger than our cache size, return */
    if (_input.webpage_len > max_size)
        return;

    /*updating the time_stamp*/
    time_stamp++;
    /* evict LFU pages to make place for this new webpage */
    while (_input.webpage_len > (max_size - cache_size)) {
        evict_lru_page(); /* evict one webpage at a time */
    }
	
    std::cout<<"Putting page "<<url<<" to cache \n";

    /* safety check */
    assert(_input.webpage_len <= (max_size - cache_size));

    webpage newpage;
    newpage.webpage_data.assign(_input.webpage_data);
    newpage.webpage_len = _input.webpage_len;

    /* insert the new webpage into the cache */
    url_to_webpage.insert(std::pair<std::string, std::pair<webpage, int32_t> >
            (url, std::pair<webpage, int32_t>(newpage, time_stamp)));

    /* adjust time_stamp_to_url<->url mapping */
    time_stamp_to_url.insert(std::pair<int32_t, std::string>(time_stamp, url));

    /* update cache_size */
    cache_size += _input.webpage_len;
}
