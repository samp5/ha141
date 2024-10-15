#include "snn_connect.h"

int get_socket() {
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

std::string pack(ConfigDict d, FunctionIdentifier flag) {
  std::ostringstream ret;
  ret << flag;
  for (const auto [k, v] : d) {
    ret << k << "\n" << v << "\n";
  }
  return ret.str();
}
