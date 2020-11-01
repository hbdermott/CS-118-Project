#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <fstream>
#include <thread>
#include <pthread.h>
#include <signal.h>
#include "crc.h"
#define BUFSIZE 1024
#define CRCSIZE 8
#define MAXCLIENTS 10

std::string directory;
CRC64 validator;

struct thread_data {
    int tid;
    int clientfd;
};

void sighandler(int i){
  std::cout << "SIGQUIT | SIGTERM received" << std::endl;
  exit(0);
}

void* service_client(void * args){
    struct thread_data *data = (struct thread_data *)args;
    struct timeval timeout;
    fd_set set;
    char buf[1024];
    int size = 0;
    std::ofstream file;

    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    std::string filename = "." + directory + "/" + std::to_string(data->tid) + ".file";
    file.open(filename);

    while(true){
      FD_ZERO(&set);
      FD_SET(data->clientfd, &set);

      int ret = select(FD_SETSIZE, &set, NULL, NULL, &timeout);

      switch(ret){
        case 0:
          file << "ERROR: Client Timeout";
          file.close();
          close(data->clientfd);
          return data;
          break;
        case -1:
          file << "ERROR: Select error";
          file.close();
          close(data->clientfd);
          return data;
          break;
        default:
          size = read(data->clientfd, &buf, 1024);
          if(size < 0){
            file << "ERROR: Read from file";
            file.close();
            close(data->clientfd);
            return data;
          }
          else if(size == 0){
            file.close();
            close(data->clientfd);
            return data;
          }
          else if (validator.getCRC((u_int8_t *)&buf, size) == 0){
            file.write(buf, 1016);
          }
          else{
            file << "ERROR: CRC Verification";
            file.close();
            close(data->clientfd);
            return data;
          }
      }
    }
    file.close();
    close(data->clientfd);
    return data;
}

int main(int argc, char *argv[])
{
  int listenfd, portno;
  int yes = 1;
  int thread_count = 1;
  pthread_t clientThreads[MAXCLIENTS];
  struct sockaddr_in addr;
  struct sockaddr_in clientAddr;

  signal(SIGQUIT, sighandler);
  signal(SIGTERM, sighandler);

  if (argc != 3){ //Making sure the length of cli arguments is 3
    fprintf(stderr, "Format: ./server <PORT> <FILE-DIR>\n");
    exit(2);
  }

  portno = atoi(argv[1]);

  if (portno < 1024 || portno > 65535){ //Make sure portno is in valid range
    fprintf(stderr, "ERROR: Invalid Port Number: %s\n", argv[1]);
    exit(2);
  }

  directory = argv[2];    //Assume directory is correct. Waiting to create fd in each individual thread
  // create a socket using TCP IP
  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    perror("failed socket fd");
    exit(1);
  }

  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0) {
    perror("setsockopt");
    exit(1);
  }

  // bind address to socket
  memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(portno);     // short, network byte order
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
 

  if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    perror("bind");
    exit(2);
  }

  // set socket to listen status
  if (listen(listenfd, 10) < 0) {
    perror("listen");
    exit(3);
  }


  while(1){
    socklen_t len = sizeof(clientAddr);
    int newfd = accept(listenfd, (struct sockaddr*)&clientAddr, &len);
    if(newfd < 0){
      perror("accept");
      exit(4);
    }
    struct thread_data data;
    data.tid = thread_count;
    data.clientfd = newfd;
    pthread_create(&clientThreads[thread_count - 1], NULL, service_client, (void*)&data);
    thread_count++;
  }

  return 0;
}
