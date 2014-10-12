GEN_SRC := webproxy.cpp base_constants.cpp base_types.cpp
RAND_SRC := rand_cache.cpp
LRU_SRC := lru_cache.cpp
MLRU_SRC := minlru_cache.cpp

GEN_OBJ := $(patsubst %.cpp,%.o, $(GEN_SRC))

THRIFT_DIR := /usr/local/include/thrift
BOOST_DIR := /usr/local/include

INC := -I$(THRIFT_DIR) -I$(BOOST_DIR)

.PHONY: all clean

all: rpc_server_lru rpc_server_minlru rpc_server_rand client

%.o: %.cpp
	$(CXX) -Wall -DHAVE_INTTYPES_H -DHAVE_NETINET_IN_H $(INC) -c $< -o $@

rpc_server_lru: webproxy_server.o $(GEN_OBJ) $(LRU_SRC)
	$(CXX) $^ -o $@ -L/usr/local/lib -lthrift -lcurl

rpc_server_minlru: webproxy_server.o $(GEN_OBJ) $(MLRU_SRC)
	$(CXX) $^ -o $@ -L/usr/local/lib -lthrift -lcurl

rpc_server_rand: webproxy_server.o $(GEN_OBJ) $(RAND_SRC)
	$(CXX) $^ -o $@ -L/usr/local/lib -lthrift -lcurl

client: client.o $(GEN_OBJ)
	$(CXX) $^ -o $@ -L/usr/local/lib -lthrift 

clean:
	$(RM) *.o webproxy_server client
