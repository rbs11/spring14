#include "webproxy.h"  // As an example
  
  #include <transport/TSocket.h>
  #include <transport/TBufferTransports.h>
  #include <protocol/TBinaryProtocol.h>
  
  using namespace apache::thrift;
  using namespace apache::thrift::protocol;
  using namespace apache::thrift::transport;
  
  using namespace Rpc;
  
  int main(int argc, char **argv) {
    boost::shared_ptr<TSocket> socket(new TSocket("localhost", 9090));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    webpage _return;
    webproxyClient client(protocol);
    transport->open();
    client.ping();
    client.httpget(_return, "www.google.com");
    client.httpget(_return, "www.google.com");
    client.httpget(_return, "www.google.com");
    client.httpget(_return, "www.google.com");
    client.httpget(_return, "www.nytimes.com");
    client.httpget(_return, "www.nytimes.com");
    client.httpget(_return, "www.nytimes.com");
    client.httpget(_return, "www.arstechnica.com");
    client.httpget(_return, "www.gsmarena.com");
    client.httpget(_return, "www.anandtech.com");
    client.httpget(_return, "www.wikipedia.org");
    client.httpget(_return, "www.youtube.com");
    client.httpget(_return, "www.apple.com");
    client.httpget(_return, "www.timesofindia.com");
    client.httpget(_return, "www.thehindu.com");
    client.httpget(_return, "www.google.com");
	client.httpget(_return, "www.nytimes.com");
	client.httpget(_return, "www.youtube.com");
	client.httpget(_return, "www.apple.com");
	client.httpget(_return, "www.timesofindia.com");
	client.httpget(_return, "www.thehindu.com");
	client.httpget(_return, "www.arstechnica.com");
	client.httpget(_return, "www.gsmarena.com");
	client.httpget(_return, "www.anandtech.com");
	client.httpget(_return, "www.wikipedia.org");


    transport->close();
  
    return 0;
  }
