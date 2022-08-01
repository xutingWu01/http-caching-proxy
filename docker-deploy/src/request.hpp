#include <stdio.h>
#include <string.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <sstream>


class Request{
public:
std::string origi_rqst;
std::string rqst_line;
std::string msg_withoutline;
std::string header;
std::string method;
std::string url;
//std::string protocol_version;
std::string domain;
std::string port;
//std::string header;
//std::string content;


public:
  Request(std::string input): origi_rqst(input){

    ParseRequestLine();
    ParseMethod();
    ParseURL();
    ParseHeader();
  }

  void ParseHeader();
  void ParseRequestLine();
  void ParseMethod();
  void ParseURL();
  //void ParseHost();

  int getLength();
  bool isChunked();
};

