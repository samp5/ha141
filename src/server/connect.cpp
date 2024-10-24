#include "connect.h"
#include <cstring>
#include <string>

#define SERVERPORT "5005"
int pack(int x) { return htonl(x); }
FunctionIdentifier match_function(char func_id_char) {
  int f_id = static_cast<int>(func_id_char) - '0';
  if (f_id < MIN_FUNC_ID || f_id > MAX_FUNC_ID) {
    return FunctionIdentifier::UNKNOWN_FUNC;
  }
  return static_cast<FunctionIdentifier>(f_id);
}
void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  } else {
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
  }
}
/*
 * Unpack a std::map<std::string, double>
 */
FunctionIdentifier unpack(char *packed_data, std::map<std::string, double> &m) {
  std::istringstream s_stream(packed_data);
  char f_id_char;
  s_stream >> f_id_char;
  FunctionIdentifier f_id = match_function(f_id_char);
  std::string k;
  std::string v;
  while (getline(s_stream, k)) {
    getline(s_stream, v);
    m.emplace(k, std::stod(v));
  }
  return f_id;
}

//    std::map<std::tuple<int, int>,
//             std::map<std::tuple<int, int>, std::map<std::string, float>>>
//    {
//      { (0,0), { (0,0), { "string", val, ... },
//                 ...
//               }
//      }
//      { (1,1), { (1,1), { "string", val, ... },
//                 ...
//               }
//      }
//      ...
//    }
//    "0,1:0.1:5"
// void initialize_from_packed_data(pySNN &network, char *packed_data) {
//   return f_id;
// }

int get_and_bind_socket() {
  int sockfd;
  struct addrinfo hints, *server_info, *p;
  // int number_of_bytes;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM; // want to use TCP to ensure it gets there
  hints.ai_flags = AI_PASSIVE;     // just use the server's IP.
  int return_value = getaddrinfo(NULL, SERVERPORT, &hints, &server_info);
  if (return_value != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(return_value));
    exit(1);
  }

  for (p = server_info; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("server: socket");
      continue;
    }
    // want to be able to reuse this, so we can set the socket option.
    int yes = 1;
    /*
     * SOL_SOCKET is a socket level for "all sockets"
     * SO_REUSEADDR says we want to bind to this socket and prevent other
     * binding while we are using it
     */
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      perror("setsockopt");
      exit(1);
    }
    // try to bind that socket.
    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("server: bind");
      continue;
    }

    break;
  }

  // can't connect :(
  if (p == NULL) {
    fprintf(stderr, "server: failed to bind\n");
    exit(2);
  }

  // we good.
  return sockfd;
}
