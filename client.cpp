#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#define BUFSIZE 1024
#define CRCSIZE 8

int main(int argc, char *argv[])
{
  int sockfd, portno;
  struct sockaddr_in serverAddr;
  char *hostname = argv[1];
  char *filename = argv[3];
  portno = (u_int16_t)argv[2];
  

  if(argc != 4){
    fprintf(stderr, "ERROR: Invalid arguments");
    exit(2);
  }

  if (portno < 1023){ //Make sure portno is in valid range
    fprintf(stderr, "ERROR: Invalid Port Number %s\n", argv[1]);
    exit(2);
  }
  // create a socket using TCP IP
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("failed socket fd");
    exit(1);
  }

  
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(portno);     // short, network byte order
  serverAddr.sin_addr.s_addr = inet_addr(hostname);

  // connect to the server
  if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
    perror("connect");
    return 2;
  }

  struct sockaddr_in clientAddr;
  socklen_t clientAddrLen = sizeof(clientAddr);
  if (getsockname(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen) == -1) {
    perror("getsockname");
    return 3;
  }

  char ipstr[INET_ADDRSTRLEN] = {'\0'};
  inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
  std::cout << "Set up a connection from: " << ipstr << ":" <<
    ntohs(clientAddr.sin_port) << std::endl;


  // send/receive data to/from connection
  bool isEnd = false;
  std::string input;
  char buf[20] = {0};
  std::stringstream ss;

  while (!isEnd) {
    memset(buf, '\0', sizeof(buf));

    std::cout << "send: ";
    std::cin >> input;
    if (send(sockfd, input.c_str(), input.size(), 0) == -1) {
      perror("send");
      return 4;
    }


    if (recv(sockfd, buf, 20, 0) == -1) {
      perror("recv");
      return 5;
    }
    ss << buf << std::endl;
    std::cout << "echo: ";
    std::cout << buf << std::endl;

    if (ss.str() == "close\n")
      break;

    ss.str("");
  }

  close(sockfd);

  return 0;
}
