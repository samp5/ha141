#ifndef PYSNN_CONNECT_H
#define PYSNN_CONNECT_H
#include "../funciton_id.h"
#include <arpa/inet.h>
#include <errno.h>
#include <map>
#include <netdb.h>
#include <netinet/in.h>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define SERVERPORT "5005"
#define SERVER "localhost"

typedef std::map<std::string, double> ConfigDict;
using AdjDict =
    std::map<std::tuple<int, int>,
             std::map<std::tuple<int, int>, std::map<std::string, float>>>;
/*
 * This is the client code for the network
 *
 * Shallow Wrapper around pySNN
 */

// clientSNN should have effectively a subset of the interface of pySNN
class clientSNN {
public:
  clientSNN(ConfigDict dict = {});
  void initialize(AdjDict &dict);
  static ConfigDict getDefaultConfig();

private:
  int get_socket();
  std::string pack(ConfigDict d, FunctionIdentifier flag);
  std::string pack(AdjDict d, FunctionIdentifier flag);
  int get_number_neuron_from_packed_data(char *packed_data);
};

#endif // !PYSNN_CONNECT_H
