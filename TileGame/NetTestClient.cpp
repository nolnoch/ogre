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
  std::string testMsg = "From client to server: Ahoy, matey!";
  std::clock_t start;
  std::size_t pos;
  double duration;
  int i;

  netMgr->initNetManager();
  netMgr->addNetworkInfo(PROTOCOL_UDP);
  netMgr->startServer();

  start = std::clock();
  do {
    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  } while (duration < 5.0f);

  //------------------------------------------------------------------

  if (netMgr->scanForActivity()) {
    std::string invite = std::string(netMgr->udpServerData.output);
    if (std::string::npos != invite.find(STR_OPEN)) {
      std::string svrAddr = invite.substr(STR_OPEN.length());

      netMgr->stopServer();
      netMgr->initNetManager();
      netMgr->addNetworkInfo(PROTOCOL_ALL, svrAddr.c_str());
      netMgr->startClient();
    }
  }

  start = std::clock();
  do {
    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  } while (duration < 10.0f);


  //<-------- Sending ---------->//

  /*
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

   */

  netMgr->close();
}



