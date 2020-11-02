#include <stdio.h>
#include <string>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include "crc.h"

int main(int argc, char *argv[])
{
  int sockfd, portno;
  struct sockaddr_in serverAddr;
  struct sockaddr_in clientAddr;
  socklen_t clientAddrLen = sizeof(clientAddr);
  struct addrinfo hints = {0}, *addrs;
  struct timeval timeout;
  fd_set set;
  CRC generator;
  char buf[1024];

  if (argc != 4){
    fprintf(stderr, "Format: ./client <HOSTNAME-OR-IP> <PORT> <FILENAME> \n");
    exit(2);
  }

  char *hostname = argv[1];
  char *filename = argv[3];
  portno = atoi(argv[2]);

  if (portno < 1024 || portno > 65535){ //Make sure portno is in valid range
    fprintf(stderr, "ERROR: Invalid Port Number: %s\n", argv[1]);
    exit(2);
  }

  // memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET; // AF_INET or AF_INET6 to force version
  hints.ai_socktype = SOCK_STREAM;
  // serverAddr.sin_family = AF_INET;
  // serverAddr.sin_port = htons(portno); // short, network byte order
  // serverAddr.sin_addr.s_addr = inet_addr(hostname);

  if (getaddrinfo(hostname, argv[2], &hints, &addrs) < 0){
    fprintf(stderr, "ERROR: Invalid hostname/port: %s\t%s\n", argv[1], argv[2]);
    exit(2);
  }
    // create a socket using TCP IP
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    perror("failed socket fd");
    exit(1);
  }
  struct addrinfo *addr = addrs;
  int arg = fcntl(sockfd, F_GETFL, NULL);
  arg |= O_NONBLOCK;
  fcntl(sockfd, F_SETFL, O_NONBLOCK);

  // connect to the server
  connect(sockfd, addr->ai_addr, addr->ai_addrlen);

  timeout.tv_sec = 10;
  timeout.tv_usec = 0;
  FD_ZERO(&set);
  FD_SET(sockfd, &set);

  int res = select(FD_SETSIZE, NULL, &set, NULL, &timeout);
  
  if(res == -1){
    fprintf(stderr, "ERROR: Client Connection Failed\n");
    close(sockfd);
    exit(3);
  }
  else if(res == 0){
    fprintf(stderr, "ERROR: Client Connection timed out\n");
    close(sockfd);
    exit(3);
  }

  arg = fcntl(sockfd, F_GETFL, NULL);
  arg &= (~O_NONBLOCK);
  fcntl(sockfd, F_SETFL, arg);
  std::ifstream file;
  file.open(filename);

  timeout.tv_sec = 10;
  timeout.tv_usec = 0;

  while (true)
  {
    FD_ZERO(&set);
    FD_SET(sockfd, &set);

    int ret = select(FD_SETSIZE, NULL, &set, NULL, &timeout);
    switch (ret){
      case 0:
        fprintf(stderr, "ERROR: Timed out while trying to write\n");
        file.close();
        close(sockfd);
        exit(3);
        break;
      case -1:
        fprintf(stderr, "ERROR: Select failed\n");
        file.close();
        close(sockfd);
        exit(3);
        break;
      default:
        file.read(buf, 1016);
        int size = file.gcount();
        uint64_t crc = htobe64(generator.get_crc_code((u_int8_t *)&buf, size));
        char crc_buf[8];
        memcpy(crc_buf, &crc, 8);
        for (int i = 0; i < 8; i++)
          buf[size + i] = crc_buf[i];
        if(write(sockfd, &buf, (size + 8)) < 0){
          fprintf(stderr, "ERROR: Couldn't write to socket!\n");
          close(sockfd);
          exit(3);
        }
        if (file.eof()){
          file.close();
          close(sockfd);
          return 0;
        }
    }
  }
  file.close();
  close(sockfd);
  return 0;
}
