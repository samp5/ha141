#ifndef PYSNN_CONNECT_H
#define PYSNN_CONNECT_H
#include "snn.hpp"
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
int get_socket();
std::string pack(ConfigDict d, FunctionIdentifier flag);

#endif // !PYSNN_CONNECT_H
