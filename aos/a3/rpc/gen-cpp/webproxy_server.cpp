// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "webproxy.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include "cache.h"
#include <sys/select.h>
#include <curl/curl.h>
#include <stdio.h>
#include <iostream>
using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using namespace  ::Rpc;
using namespace std;

#define KB	1024
#define	 MB	1024*KB
#define	 CACHE_SZ 500*KB
struct wd_in {
  size_t size;
  size_t len;
  char *data;
};

class webproxyHandler : virtual public webproxyIf {
 public:

	int32_t hit_count;
	int32_t total_count;
	int32_t hit_bytes_count;
	int32_t total_bytes_count;

	webproxyHandler()
	{
	// Your initialization goes here
		hit_count = 0;
		total_count = 0;
		hit_bytes_count = 0;
		total_bytes_count = 0;
		set_cache_size(CACHE_SZ);
	}

	int32_t ping()
	{
		// Your implementation goes here
		printf("ping\n");
		return 0;
	}
	static size_t write_data(void *buffer, size_t size,
							 size_t nmemb, void *userp)
	{
		struct wd_in *wdi = (struct wd_in *)userp;

		while(wdi->len + (size * nmemb) >= wdi->size)
		{
			/* check for realloc failing in real code. */
			wdi->data = (char*)realloc(wdi->data, wdi->size*2);
			wdi->size*=2;
		}
		memcpy(wdi->data + wdi->len, buffer, size * nmemb);
		wdi->len+=size*nmemb;

		return size * nmemb;
	}

	void httpget(webpage& webpg_req, const std::string& url)
	{
		// Your implementation goes here
		CURL *curl;
		CURLcode res;
		struct wd_in wdi;
		string cur_url;

		cur_url.assign(url);

		if (get_cached_page(cur_url, webpg_req) == 0)
		{
			cout<<"URL "<<cur_url<<" found in cache"<<"\n";
			hit_count++;
			hit_bytes_count += webpg_req.webpage_len;
			total_bytes_count += wdi.len;
			total_count++;
			return;
		}

		memset(&wdi, 0, sizeof(wdi));
		/* Get a curl handle.  Each thread will need a unique handle. */
		curl = curl_easy_init();

		if(NULL != curl)
		{
			wdi.size = 1024;
			/* Check for malloc failure in real code. */
			wdi.data = (char*)malloc(wdi.size*1000);
			std::cout<<"URL is "<<cur_url<<"\n";
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

			/* "write_data" function to call with returned data. */
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

			/* userp parameter passed to write_data. */
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &wdi);

			/* Actually perform the query. */
			res = curl_easy_perform(curl);

			/* Check the return value and do whatever. */
			std::cout<<"post length "<<wdi.len<<"\n";
			/* Clean up after ourselves. */
			curl_easy_cleanup(curl);

			webpg_req.webpage_len = wdi.len;
			webpg_req.webpage_data.assign(wdi.data);
			//webpg_req.webpage_data = wdi.data;

			add_to_cache(cur_url, webpg_req);
			total_bytes_count += wdi.len;
			total_count++;

		}
		else
		{
			fprintf(stderr, "Error: could not get CURL handle.\n");
			exit(EXIT_FAILURE);
		}

		/* Now wdi.data has the data from the GET and wdi.len is the length
		 of the data available, so do whatever. */

		/* Write the content to stdout. */
		//write(STDOUT_FILENO, wdi.data, wdi.len);

		/* cleanup wdi.data buffer. */
		free(wdi.data);
		return;
	}

	void get_webcache_stats(cache_stat& _return)
	{
		// Your implementation goes here
		printf("get_webcache_stats\n");
		_return.hit_count = hit_count;
		_return.total_bytes_count = total_bytes_count;
		_return.total_count = total_count;
		_return.hit_bytes_count = hit_bytes_count;
	}

	void reset_webcache_stats()
	{
		// Your implementation goes here
		printf("reset_webcache_stats\n");
	}
 };

int main(int argc, char **argv)
{
	int port = 9090;
	shared_ptr<webproxyHandler> handler(new webproxyHandler());
	shared_ptr<TProcessor> processor(new webproxyProcessor(handler));
	shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
	shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
	shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

	TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
	server.serve();
	return 0;
}

