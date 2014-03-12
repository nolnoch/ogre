/*
 * NetTestServer.cpp
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
  std::string testMsg = "From server to client; here's hoping you get this.";
  std::clock_t start;
  double duration;
  int i;

  netMgr->initNetManager();
  netMgr->addNetworkInfo();
  netMgr->startServer();

  //<-------- Receiving ---------->//
  std::cout << "\n\nPolling...\n\n" << std::endl;

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

  //<-------- Sending ---------->//
  netMgr->messageClients(testMsg.c_str(), testMsg.length());

  std::cout << "\n\nMessage sent!\n" << std::endl;

  /*
  start = std::clock();
  do {
    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  } while (duration < 30.0f);
  */

  netMgr->close();
}



