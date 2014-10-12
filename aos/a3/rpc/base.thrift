#!/usr/local/bin/thrift --gen cpp

namespace cpp Rpc

struct webpage {
	1: string webpage_data,
	2: i32 webpage_len,
}

struct cache_stat {
	1: i32 hit_count,
	2: i32 total_count,
	3: i32 hit_bytes_count,
	4: i32 total_bytes_count,
}

exception error {
	1: i32 what,
	2: string why
}

service webproxy {
	i32 ping(),
	webpage httpget(1:string url) throws (1:error err),
	cache_stat get_webcache_stats(),
	void reset_webcache_stats(),
}
