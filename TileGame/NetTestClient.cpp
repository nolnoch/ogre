/*
 * NetTestClient.cpp
 *
 *  Created on: Mar 11, 2014
 *      Author: nolnoch
 */

#include <iostream>
#include <cstring>
#include <cstdio>
#include <ctime>
#include "NetManager.h"

int main(int argc, char **argv) {
  NetManager *netMgr = new NetManager();
  std::string host = "192.168.1.8";
  std::string testMsg = "From client to server: Ahoy, matey!";
  std::clock_t start;
  double duration;
  int i;


  //<-------- Connecting ---------->//

  netMgr->initNetManager();
  netMgr->addNetworkInfo(PROTOCOL_ALL, 51215, host.c_str());
  netMgr->startClient();


  start = std::clock();
  do {
    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  } while (duration < 5.0f);


  //<-------- Sending ---------->//

  netMgr->messageServer(PROTOCOL_ALL, testMsg.c_str(), testMsg.length());

  std::cout << "\nMessage sent!\n\n" << std::endl;


  //<-------- Receiving ---------->//

  std::cout << "Polling...\n\n" << std::endl;

  if (!netMgr->pollForActivity(15000))
    std::cout << "I got nothin'." << std::endl;
  else {
    std::cout << "Received messages:\n" << std::endl;

    std::cout << netMgr->tcpServerData.output << std::endl;
    std::cout << netMgr->udpServerData.output << std::endl;
  }

  std::cout << "\n\nTest complete.\n" << std::endl;

  netMgr->close();
}



