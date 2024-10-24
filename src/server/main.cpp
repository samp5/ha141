#include "connect.h"
#include "remote_snn.hpp"
#include "utils.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#define BUFF_SIZE 10000
#define NUMBER_INPUT_NEURONS_CONST 784

/*
 * This is the server main function
 *
 * The high-level function of this program is to do the following:
 * 1. Recieve information about how to construct a network (A ConfigDict)
 * 2. Recieve an adjacency dictionary about how to construct network edges (An
 * AdjDict)
 * 3. Recieve data to run the network on and run the network
 * 4. Wait for a request to send data back to client
 *
 */

void recieve_initialization_parameters(int sockfd, char **packed_adj_list,
                                       int &number_input_neruons) {
  struct sockaddr client_addr;
  socklen_t sin_size = sizeof(client_addr);
  int new_fd = accept(sockfd, &client_addr, &sin_size);
  if (new_fd == -1) {
    perror("accept");
    exit(1);
  }

  char *buffer = new char[BUFF_SIZE];
  int bytes_recv = recv(new_fd, buffer, BUFF_SIZE, 0);
  if (bytes_recv == -1 || bytes_recv == BUFF_SIZE) {
    perror("recv");
    exit(1);
  }
  buffer[bytes_recv] = '\0';
  if (match_function(buffer[0]) != FunctionIdentifier::Initiaize) {
    perror("Second function recieved did not initialize the network, cannot "
           "continue");
    exit(1);
  }
  *packed_adj_list = buffer + 1;

  // would have to call a separate recv funcition
  // I'm pretty sure that the passed number would get swallowed up in our char
  // buffer so I would have to make several writes
  // 1. amount of packed bytes
  // 2. the actual bytes
  // 3. the number of input neurons
  number_input_neruons = NUMBER_INPUT_NEURONS_CONST;
}
// void initialize_network(pySNN &network, int sockfd) {
//   // initialize_from_packed_data(network, buffer + 1);
// }
//
std::map<std::string, double> process_config_map(int sockfd) {
  struct sockaddr client_addr;
  socklen_t sin_size = sizeof(client_addr);
  int new_fd = accept(sockfd, &client_addr, &sin_size);
  if (new_fd == -1) {
    perror("accept");
    exit(1);
  }

  char *buffer = new char[BUFF_SIZE];
  int bytes_recv = recv(new_fd, buffer, BUFF_SIZE, 0);
  if (bytes_recv == -1 || bytes_recv == BUFF_SIZE) {
    perror("recv");
    exit(1);
  }
  buffer[bytes_recv] = '\0';
  std::map<std::string, double> recieved_map;
  FunctionIdentifier f_id = unpack(buffer, recieved_map);
  if (f_id != FunctionIdentifier::Construct) {
    perror("First function recieved was not a constructor, cannot continue");
    exit(1);
  }

  std::cout << "Recieved ConfigDict: \n";
  print_map(recieved_map);
  return recieved_map;
}

int main(void) {
  int sockfd = get_and_bind_socket();
  if (listen(sockfd, 1) == -1) {
    perror("listen");
    exit(1);
  }

  std::cout << "Waiting for connections...\n";

  // struct sockaddr client_addr;
  // socklen_t sin_size;
  // int new_fd;

  // get constructor
  rpcSNN network(process_config_map(sockfd));

  char *packed_adj_list;
  int number_input_neruons;
  recieve_initialization_parameters(sockfd, &packed_adj_list,
                                    number_input_neruons);
  network.initialize(packed_adj_list, number_input_neruons);

  // main loop

  // while (1) {
  //   sin_size = sizeof(client_addr);
  //   new_fd = accept(sockfd, &client_addr, &sin_size);
  //   if (new_fd == -1) {
  //     perror("accept");
  //     exit(1);
  //   }
  //
  //   char *buffer = new char[10000];
  //   int bytes_recv = recv(new_fd, buffer, 10000, 0);
  //   if (bytes_recv == -1) {
  //     perror("recv");
  //     exit(1);
  //   }
  //   buffer[bytes_recv] = '\0';
  //   std::map<std::string, double> recieved_map;
  //   FunctionIdentifier f_id = unpack(buffer, recieved_map);
  //
  //   switch (f_id) {
  //   case FunctionIdentifier::Construct: {
  //     perror("Recieved construct function call after network is already "
  //            "constructed, ending.");
  //     exit(1);
  //     break;
  //   }
  //   case FunctionIdentifier::Initiaize: {
  //     perror("Recieved Initiaize function call after network is already "
  //            "initiaized, ending.");
  //     exit(1);
  //     break;
  //     break;
  //   }
  //   case FunctionIdentifier::BatchReset: {
  //     break;
  //   }
  //   case FunctionIdentifier::GetActivation: {
  //     break;
  //   }
  //   case FunctionIdentifier::RunBatch: {
  //   }
  //   case FunctionIdentifier::UNKNOWN_FUNC:
  //     std::cout << "Function could not be determined";
  //     break;
  //   }
  // }
}
