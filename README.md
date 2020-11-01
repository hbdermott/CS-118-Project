# CS-118-Project
A socket based client/server project to practice CRC encoding/decoding+

server.cpp:
  A server that accepts up to 10 clients by using multithreading. Clients are
  accepted and a new thread is generated to handle reading. All input from client
  is read into a 1024 byte sized buffer and is checked to see if the CRC is valid
  by calling the method from the CRC64 class. If valid, the input is written to a
  file named dir/#.file where dir is the specified directory and # is the order of
  client connection starting at 1.
