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
  int i = 0;

  netMgr->initNetManager();
  netMgr->addNetworkInfo(PROTOCOL_UDP);
  netMgr->startServer();


  //----------------------------------------------------------------------

  netMgr->multiPlayerInit();

  start = std::clock();
  do {
    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  } while (duration < 10.0f);

  netMgr->scanForActivity();


  //<-------- Connecting ---------->//
  /*


  start = std::clock();
  do {
    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  } while (duration < 10.0f);


  std::cout << "\n\nPolling for clients...\n\n" << std::endl;

  if (!netMgr->pollForActivity(15000)) {
    std::cout << "Nobody loves me." << std::endl;
    std::cout << "TCP Count: " << netMgr->getTCPClients() << std::endl;
    std::cout << "UDP Count: " << netMgr->getUDPClients() << std::endl;
  } else {
    std::cout << "New Client(s)!" << std::endl;
    std::cout << "TCP Count: " << netMgr->getTCPClients() << std::endl;
    std::cout << "UDP Count: " << netMgr->getUDPClients() << std::endl;
  }


  //<-------- Receiving ---------->//

  std::cout << "\n\nPolling for data...\n\n" << std::endl;

  if (!netMgr->pollForActivity(15000))
    std::cout << "I got nothin'." << std::endl;
  else {
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
  }


  //<-------- Sending ---------->//

  netMgr->messageClients(testMsg.c_str(), testMsg.length());

  std::cout << "\n\nMessage sent!\n" << std::endl;


  start = std::clock();
  do {
    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  } while (duration < 5.0f);


  std::cout << "\n\nTest complete.\n" << std::endl;

   */

  netMgr->close();
}



