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
    std::string _return;
    webproxyClient client(protocol);
    transport->open();
    client.ping();
    client.httpget(_return, "http://www.nytimes.com");
    transport->close();
  
    return 0;
  }
