#include "../pybind/snn.hpp"
#include "connect.h"
#include "utils.h"
#include <cstdio>

int main(void) {
  int sockfd = get_and_bind_socket();
  if (listen(sockfd, 1) == -1) {
    perror("listen");
    exit(1);
  }

  std::cout << "waiting for connections\n";

  struct sockaddr client_addr;
  socklen_t sin_size;
  int new_fd;
  while (1) {
    sin_size = sizeof(client_addr);
    new_fd = accept(sockfd, &client_addr, &sin_size);
    if (new_fd == -1) {
      perror("accept");
      continue;
    }

    char *buffer = new char[10000];
    int bytes_recv = recv(new_fd, buffer, 10000, 0);
    if (bytes_recv == -1) {
      perror("recv");
      continue;
    }
    buffer[bytes_recv] = '\0';
    std::map<std::string, float> recieved_map;
    FunctionIdentifier f_id = unpack(buffer, recieved_map);
    if (f_id != FunctionIdentifier::Construct) {
      perror("First function recieved was not a constructor, cannot continue");
      exit(1);
    }
    // switch (f_id) {
    // case PRINT_MAP: {
    //   bool res = print_map(recieved_map);
    //   if (res) {
    //     std::cout << "Successfully printed map, sending true to client\n";
    //   } else {
    //     std::cout << "Failed to print map, sending false to client\n";
    //   }
    //
    //   if (send(new_fd, &res, sizeof(res), 0) == -1) {
    //     perror("send");
    //   }
    //   close(new_fd);
    //   break;
    // }
    // case SUM_MAP: {
    //   int res = sum_map(recieved_map);
    //   std::cout << "Successfully summed map, sending " << res << " to
    //   client\n"; if (send(new_fd, &res, sizeof(res), 0) == -1) {
    //     perror("send");
    //   }
    //   close(new_fd);
    //   break;
    // }
    // case REMOVE_DUPLICATES:
    // case UNKNOWN_FUNC:
    //   std::cout << "Function could not be determined";
    //   break;
    // }
  }
}
