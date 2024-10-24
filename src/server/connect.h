#ifndef CONNECT_H
#define CONNECT_H
#include "../funciton_id.h"
#include "remote_snn.hpp"
#include <arpa/inet.h>
#include <cstdio>
#include <map>
#include <netdb.h>
#include <netinet/in.h>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void *get_in_addr(struct sockaddr *sa);
FunctionIdentifier unpack(char *packed_data, std::map<std::string, double> &m);
void initialize_from_packed_data(rpcSNN &network, char *packed_data);
FunctionIdentifier match_function(char func_id_char);
int get_and_bind_socket();
int pack(int x);

#endif // !CONNECT_H
