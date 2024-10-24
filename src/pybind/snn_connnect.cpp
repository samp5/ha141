#include "../network.hpp"
#include "snn_connect.h"
#include <cstdio>
#include <iostream>
#include <unistd.h>

/**
 * @brief Transform the coordinate of a 2D array into a 1D index.
 *
 *
 * Returns the index as if the rows of the 2D array were laid end to end
 *
 * Given (x,y) such that (x, y) identify a point in an n by m matrix,
 * return an index of a 1D matrix of n * m elements.
 *
 * (0,0) => 0
 * (m, n) => m*n - 1.
 *
 * For (a, b) => j, (c,d) => k, if a > c, j > k.
 *
 * @param pair coordinate (x,y) in a tuple
 * @param maxLayer the value of m (number of rows)
 * @return index of the 1D array
 */
int getIndex(const std::tuple<int, int> &pair, int numCols) {
  int row = std::get<0>(pair);
  int col = std::get<1>(pair);

  int index = col + numCols * row;
  return index;
}

int clientSNN::get_socket() {
  int sockfd;
  struct addrinfo hints, *server_info, *p;
  int number_of_bytes;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  /* hints points to an addrinfo struct that specifies criteria for selecting
   * the socket address structures returned in the list server_info
   *
   * This function gives us internet addressses that we can call in bind()
   */
  int return_value = getaddrinfo(SERVER, SERVERPORT, &hints, &server_info);
  if (return_value != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(return_value));
    exit(1);
  }

  // server_info holdsa linked-list of addresses, and we want to connect to the
  // first one we can
  for (p = server_info; p != NULL; p = p->ai_next) {
    // try to make a socket
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      // we couldn't make this socket :(
      perror("client: socket");
      // try the next one
      continue;
    }
    // Try to connect to that socket.
    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      // we didn't connect :(
      close(sockfd);
      perror("client: connect");
      // try the next one
      continue;
    }

    // we have a valid socket and can stop iterating through.
    break;
  }

  // not a valid sockaddr,can't connect.
  if (p == NULL) {
    fprintf(stderr, "client: failed to connect\n");
    exit(2);
  }

  // we good.
  return sockfd;
}

std::string clientSNN::pack(ConfigDict d, FunctionIdentifier flag) {
  std::ostringstream ret;
  ret << flag;
  for (const auto &[k, v] : d) {
    ret << k << "\n" << v << "\n";
  }
  return ret.str();
}

std::string clientSNN::pack(AdjDict d, FunctionIdentifier flag) {
  std::ostringstream ret;
  ret << flag;

  size_t num_cols = std::get<1>(d.rbegin()->first) + 1;

  for (const auto &[origin, dest_map] : d) {
    ret << getIndex(origin, num_cols); // "0"
    if (dest_map.empty()) {
      continue;
    }
    for (const auto &[destination, attribute_map] : dest_map) {
      ret << "," << getIndex(destination, num_cols); // "0,1"
      ret << ":";                                    // "0,1:"
      if (attribute_map.count("weight")) {
        ret << attribute_map.at("weight"); // "0,1:0.1"
      }
      ret << ":"; // "0,1:0.1:"
      if (attribute_map.count("delay")) {
        ret << attribute_map.at("delay"); // "0,1:0.1:5"
      }
    }
    ret << "\n";
  }
  return ret.str();
}

//  bool print_map(const std::map<std::string, int> &m) {
//   // pack the data into a string
//   std::string data = pack(m, FunctionIdentifier::PRINT_MAP);
//   int socketfd = get_socket();
//
//   if (send(socketfd, data.data(), sizeof(char) * data.length(), 0) == -1) {
//     perror("send");
//     close(socketfd);
//     exit(0);
//   }
//   int buf[1];
//   int bytes_recieved = recv(socketfd, &buf, 1, 0);
//   if (bytes_recieved == -1) {
//     perror("recv");
//     exit(1);
//   }
//
//   bool result = buf[0];
//
//   close(socketfd);
//   return result;
// }
clientSNN::clientSNN(ConfigDict dict) {
  // pack the data into a string
  std::string data = pack(dict, FunctionIdentifier::Construct);
  int socketfd = get_socket();

  if (send(socketfd, data.data(), sizeof(char) * data.length(), 0) == -1) {
    perror("failed to send config dict");
    close(socketfd);
    exit(0);
  }
  close(socketfd);
  std::cout << "sent config dict to server \n";
}

void clientSNN::initialize(AdjDict &dict) {
  // pack the data into a string
  std::string data = pack(dict, FunctionIdentifier::Initiaize);
  int socketfd = get_socket();

  if (send(socketfd, data.data(), sizeof(char) * data.length(), 0) == -1) {
    perror("failed to send adj dict");
    close(socketfd);
    exit(0);
  }
  close(socketfd);
  std::cout << "sent adj dict to server \n";
}
ConfigDict clientSNN::getDefaultConfig() {

  ConfigDict dict = {{"neuron_count", 0},
                     {"input_neuron_count", 0},
                     {"group_count", 1},
                     {"edge_count", 0},
                     {"refractory_duration", 5},
                     {"initial_membrane_potential", -6.0},
                     {"activation_threshold", -5.0},
                     {"refractory_membrane_potential", -7.0},
                     {"tau", 100.0},
                     {"max_latency", 10},
                     {"max_synapse_delay", 2},
                     {"min_synapse_delay", 1},
                     {"max_weight", 10.0},
                     {"poisson_prob_of_success", 0.8},
                     {"debug_level", LogLevel::NONE},
                     {"limit_log_size", true},
                     {"show_stimulus", false},
                     {"time_per_stimulus", 200},
                     {"seed", -1},
                     {"rpc", false}};
  return dict;
}
