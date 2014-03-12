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

  netMgr->initNetManager();
  netMgr->addNetworkInfo(PROTOCOL_ALL, 51215, host.c_str());
  netMgr->startClient();


  //<-------- Sending ---------->//
  netMgr->messageServer(testMsg.c_str(), testMsg.length());

  std::cout << "\nMessage sent!\n\n" << std::endl;
  std::cout << "Waiting...\n\n" << std::endl;

  start = std::clock();
  do {
    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  } while (duration < 20.0f);


  //<-------- Receiving ---------->//
  std::cout << "Polling...\n\n" << std::endl;

  netMgr->pollForActivity(20000);

  std::cout << "Received messages:\n" << std::endl;

  for (i = 0; i < netMgr->tcpClientData.size(); i++) {
    if (netMgr->tcpClientData[i]->updated) {
      std::cout << netMgr->tcpClientData[i]->output << std::endl;
    }
  }

  for (i = 0; i < netMgr->udpClientData.size(); i++) {
    if (netMgr->udpClientData[i]->updated) {
      std::cout << netMgr->udpClientData[i]->output << std::endl;
    }
  }

  std::cout << "\n\nTest complete.\n" << std::endl;

  netMgr->close();
}



