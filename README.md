# CS-118-Project
A socket based client/server project to practice CRC encoding/decoding+
Hunter Dermott 404982180


LIBRARIES
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
#include <thread>

server.cpp:
  ./server <PORT-NO> <DIR-NAME>
  A server that accepts up to 10 clients by using C++11 threads. Clients are
  accepted and a new thread is generated to handle reading. All input from client
  is read into a 1024 byte sized buffer and is checked to see if the CRC is valid
  by calling the get_crc_code method from the CRC class. If valid, the input is written 
  to a file named dir/#.file where dir is the specified directory and # is the order of
  client connection starting at 1. After 10 seconds of not receiving data from the client
  the server times out and disconnects from that client.

  During the development of this, I had a lot of issues with choosing between select or
  multithreading. Since I knew I wanted a timer and select had one built in it seemed like
  a good option. For simplicity sake I decided to create a new thread whenever a client joined.
  I initially used POSIX threads, but for some reason this caused errors were the file
  wouldn't write properly. After switching to C++11 threads the problem was fixed.

client.cpp:
  ./client <HOSTNAME> <PORT-NO> <FILENAME>
  A client that connects to a server at the given hostname and port number. If a connection
  cannot be made within 10 seconds it times out and exits. If a connection is made, the client
  starts reading from the given file 1016 byte chunks at a time. A CRC code is then generated
  for this chunk and appened to the back to form a 1024 byte chunk. This is then sent to the
  server via the socket.

  The main issue I had with creating the client was trying to figure out how to use the nonblocking
  select to detect if the client connected to the server within the given time frame. Once I figured
  out how to switch to nonblocking and back, there wasnt much problem. Another big issue was trying
  to resolve the localhost to an ip, but this was solved by using the addrinfo features.

crc.cpp:
  A CRC class that generates a CRC 64-bit lookup table and returns a crc value for a given byte
  stream.

  I did not really have many problems in the implementation of this code.


Awknowledgements:
CRC: http://www.sunshine2k.de/articles/coding/crc/understanding_crc.html

Network programming: https://beej.us/guide/bgnet/pdf/bgnet_usl_c_1.pdf

Client Select Timeout: https://stackoverflow.com/questions/2597608/c-socket-connection-timeout