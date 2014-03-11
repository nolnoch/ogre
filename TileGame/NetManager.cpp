/*
 * NetManager.cpp
 *
 *  Created on: Mar 7, 2014
 *      Author: nolnoch
 */

#include "NetManager.h"



/* ****************************************************************************
 * Constructors/Destructors
 */

/**
 * Initialize changeable values to defaults. Nothing special.
 */
NetManager::NetManager():
netStatus(NET_UNINITIALIZED),
nextUDPChannel(CHANNEL_DEFAULT),
forceClientRandomUDP(true),
acceptNewClients(true),
socketNursery(0),
netProtocol(0),
netPort(0)
{
}

/**
 * Standard destruction. Calls close().
 */
NetManager::~NetManager() {
  if (netStatus > NET_INITIALIZED)
    close();
  SDLNet_FreeSocketSet(socketNursery);
  SDLNet_Quit();
}



/* ****************************************************************************
 * Public
 */


/**
 * Initializes the SDL library if it has not started already, followed by the
 * SDL_net library. If both succeed, the internal SocketSet is allocated, and
 * the state is set to NET_INITIALIZED.
 * @return True on success, false on failure.
 */
bool NetManager::initNetManager() {
  bool ret = true;

  if (SDL_Init(0)==-1) {
    printf("SDL_Init: %s\n", SDL_GetError());
    ret = false;
  } else if (SDLNet_Init()==-1) {
    printf("SDLNet_Init: %s\n", SDLNet_GetError());
    ret = false;
  } else {
    socketNursery = SDLNet_AllocSocketSet(SOCKET_ALL_MAX);

    if (!socketNursery) {
      std::cout << "SDL_net: Unable to allocate SocketSet." << std::endl;
      std::cout << SDLNet_GetError() << std::endl;
      ret = false;
    } else
      netStatus |= NET_INITIALIZED;
  }

  return ret;
}

/**
 * @brief Required to set TCP/UDP, port, and optional host.
 *
 * Allows user to set preferred protocol, port, and optional host. If a host is
 * given, it is assumed to be the server, and a client initialization is
 * expected. If no host is given, only a server initialization is possible and
 * will be expected.  Protocol and port are given default values if either or
 * both are not specified.
 * @param protocol Desired protocols for server or client. Default: ALL.
 * @param port Desired port for server or client. Default: 51215
 * @param host Host server if starting client. Default: NULL (begin server).
 */
void NetManager::addNetworkInfo(Protocol protocol, Uint16 port, const char *host) {
  if (statusCheck(NET_INITIALIZED)) {
    std::cout << "NetManager: Must initNetManager before proceeding." << std::endl;
    return;
  }

  setProtocol(protocol);
  setPort(port ? : PORT_DEFAULT);
  if (host)
    setHost(host);

  netStatus |= NET_WAITING;
}

/**
 * @brief Launch a server to listen on given/default port over
 * given/default protocols.
 *
 * Warns if host server was specified but proceeds with launch.
 * @return True on success, false on failure.
 */
bool NetManager::startServer() {
  bool ret = true;

  if (statusCheck(NET_WAITING)) {
    std::cout << "NetManager: Must addNetworkInfo before starting server." << std::endl;
    return false;
  } else if (netStatus & NET_CLIENT) {
    std::cout << "NetManager: Client already started. May not start server." << std::endl;
    return false;
  } else if (!netHost.empty()) {
    std::cout << "NetManager: Host was specified. Are you sure you want to "
        "start a server?" << std::endl;
  }

  if (netProtocol & PROTOCOL_TCP) {
    ret = openServer(PROTOCOL_TCP, netPort);
  }
  if (netProtocol & PROTOCOL_UDP) {
    ret = ret || openServer(PROTOCOL_UDP, netPort);
  }

  return ret;
}

/**
 * @brief Launch a client on given/default port over given/default protocols
 * and connected to given host server.
 *
 * Fails if no host server was given in addNetworkInfo or addHost.
 * @return True on success, false on failure.
 */
bool NetManager::startClient() {
  bool ret = false;

  if (statusCheck(NET_WAITING)) {
    std::cout << "NetManager: Must addNetworkInfo before starting client." << std::endl;
    return false;
  } else if (netStatus & NET_CLIENT) {
    std::cout << "NetManager: Server already started. May not start client." << std::endl;
    return false;
  } else if (netHost.empty()) {
    std::cout << "NetManager: Must specify a host to start a client." << std::endl;
    return false;
  }

  if (netProtocol & PROTOCOL_TCP) {
    ret = openClient(PROTOCOL_TCP, netHost, netPort);
  }
  if (netProtocol & PROTOCOL_UDP) {
    ret = ret || openClient(PROTOCOL_UDP, netHost, netPort);
  }

  return ret;
}

/**
 * @brief Poll for activity on all TCP and UDP sockets.
 *
 * If activity is detected, it will be automatically handled according to its
 * protocol and the server or client configuration. New clients and data will
 * be processed before this function returns. If the return is \b true, the
 * <em> user should immediately scan the external MessageInfo bins </em> for
 * newly output data.
 * @param timeout_ms Time in milliseconds to block and poll. Default: 5 seconds.
 * @return True for activity, false for no activity.
 */
bool NetManager::pollForActivity(Uint32 timeout_ms) {
  if (statusCheck(NET_UDP_OPEN, NET_TCP_ACCEPT)) {
    std::cout << "NetManager: No established TCP or UDP sockets to poll." << std::endl;
    return false;
  }

  return checkSockets(timeout_ms);
}

/**
 * @brief Scan once for activity on all TCP and UDP sockets.
 *
 * This calls pollForActivity with a time of 0 milliseconds (instant).
 * @return True for activity, false for no activity.
 * @see pollForActivity()
 */
bool NetManager::scanForActivity() {
  return pollForActivity(0);
}

/**
 * @brief Send a single message to all clients.
 *
 * Must be running as a server to call this function. If no arguments are given,
 * it will pull from each client's MessageBuffer \b input field.
 * @param buf Manually given data buffer. Default: NULL.
 * @param len Length of given buffer. Default: 0.
 */
void NetManager::messageClients(char *buf, int len) {
  int i;

  if (statusCheck(NET_SERVER)) {
    std::cout << "NetManager: No server running, and thus no clients to message." << std::endl;
    return;
  }

  if (netServer.protocols & PROTOCOL_TCP) {
    for (i = 0; i < tcpClients.size(); i++) {
      sendTCP(tcpSockets[tcpClients[i]->tcpSocketIdx], buf, len);
    }
  }
  if (netServer.protocols & PROTOCOL_UDP) {
    UDPpacket *pack = craftUDPpacket(buf, len);

    if (pack)
      for (i = 0; i < udpClients.size(); i++) {
        pack->len = len;
        sendUDP(udpSockets[udpClients[i]->udpSocketIdx],
            udpClients[i]->udpChannel, pack);
      }
  }
}

/**
 * @brief Send a single message to the server.
 *
 * Must be running as a client to call this function. If no argumetns are given,
 * it will pull from the server's MessageBuffer \b input field.
 * @param buf Manually given data buffer. Default: NULL.
 * @param len Length of given buffer. Default: 0.
 */
void NetManager::messageServer(char *buf, int len) {
  if (statusCheck(NET_CLIENT)) {
    std::cout << "NetManager: No client running, and thus no server to message." << std::endl;
    return;
  }

  if (netServer.protocols & PROTOCOL_TCP) {
    sendTCP(tcpSockets[netServer.tcpSocketIdx], buf, len);
  }
  if (netServer.protocols & PROTOCOL_UDP) {
    UDPpacket *pack = craftUDPpacket(buf, len);
    if (pack)
      sendUDP(udpSockets[netServer.udpSocketIdx], netServer.udpChannel, pack);
  }
}

/**
 * @brief Send a single message to a single client over a single protocol.
 *
 * Must be running as a server, and all fields must be provided by the user.
 * This will send the given message to the specified client using the specified
 * protocol.
 * @param protocol TCP or UDP, given by the PROTOCOL_XXX enum value.
 * @param clientDataIdx Index of the client into the tcp/udp ClientData vector.
 * @param buf Manually given data buffer.
 * @param len Length of the given buffer.
 * @see messageClients()
 */
void NetManager::messageClient(Protocol protocol, int clientDataIdx, char *buf, int len) {
  if (statusCheck(NET_SERVER)) {
    std::cout << "NetManager: No server running, and thus no clients to message." << std::endl;
    return;
  }

  if (protocol & PROTOCOL_TCP) {
    TCPsocket client = tcpSockets[tcpClients[clientDataIdx]->tcpSocketIdx];
    sendTCP(client, buf, len);
  } else if (protocol & PROTOCOL_UDP) {
    UDPpacket *pack = craftUDPpacket(buf, len);
    ConnectionInfo *cInfo = udpClients[clientDataIdx];
    UDPsocket client = udpSockets[cInfo->udpSocketIdx];
    if (pack)
      sendUDP(client, cInfo->udpChannel, pack);
  }
}

/**
 * @brief Removes an established client from a running server.
 *
 * Must be running as a server, and must give a connected client. May choose to
 * drop the client from TCP, UDP, or both.
 * @param protocol TCP, UDP, or ALL; given by PROTOCOL_XXX enum value.
 * @param clientDataIdx Index of the client into the tcp/udp ClientData vector.
 */
void NetManager::dropClient(Protocol protocol, int clientDataIdx) {
  if (statusCheck(NET_SERVER)) {
    std::cout << "NetManager: No server running, and thus no clients to drop." << std::endl;
    return;
  }

  if (protocol & PROTOCOL_TCP) {
    TCPsocket client = tcpSockets[tcpClients[clientDataIdx]->tcpSocketIdx];
    unwatchSocket(&client);
    closeTCP(client);
    tcpSockets.erase(tcpSockets.begin() + clientDataIdx);
  }
  if (protocol & PROTOCOL_UDP) {
    ConnectionInfo *cInfo = udpClients[clientDataIdx];
    UDPsocket client = udpSockets[cInfo->udpSocketIdx];
    unbindUDPSocket(client, cInfo->udpChannel);

    // TODO Implement reclaimable channels through bitmap or 2d array?
  }
}

/**
 * @brief Shut down the running server on any or all protocols.
 *
 * Must be running as a server to call this function. If after completing the
 * requested removal there are no active protocols, all data structures will
 * be emptied, freed, and reset to default values. The state of the instance
 * will return to NET_INITIALIZED, allowing for start of new client or server
 * after another call to addNetworkInfo().
 * @param protocol TCP, UDP, or all; given by PROTOCOL_XXX enum value
 * @see resetManager()
 * @see close()
 */
void NetManager::stopServer(Protocol protocol) {
  int i;

  if (statusCheck(NET_SERVER)) {
    std::cout << "NetManager: There's no server running, dummy." << std::endl;
    return;
  }

  if (protocol & PROTOCOL_TCP) {
    for (i = 0; i < tcpClients.size(); i++) {
      TCPsocket client = tcpSockets[i];
      unwatchSocket(&client);
      closeTCP(client);
      tcpSockets.erase(tcpSockets.begin() + i);
    }
    netServer.protocols ^= PROTOCOL_TCP;
  }
  if (protocol & PROTOCOL_UDP) {
    for (i = 0; i < udpClients.size(); i++) {
      ConnectionInfo *cInfo = udpClients[i];
      UDPsocket client = udpSockets[cInfo->udpSocketIdx];
      unbindUDPSocket(client, cInfo->udpChannel);
    }
    for (i = udpSockets.size() - 1; i > 0; i--) {
      unwatchSocket(&udpSockets[i]);
      closeUDP(udpSockets[i]);
      udpSockets.pop_back();
    }
    netServer.protocols ^= PROTOCOL_UDP;
  }

  if (!netServer.protocols)
    resetManager();
}

/**
 * @brief Shut down the running client on any or all protocols.
 *
 * Must be running as a client to call this function. If after completing the
 * requested removal there are no active protocols, all data structures will
 * be emptied, freed, and reset to default values. The state of the instance
 * will return to NET_INITIALIZED, allowing for start of new client or server
 * after another call to addNetworkInfo().
 * @param protocol TCP, UDP, or all; given by PROTOCOL_XXX enum value
 * @see resetManager()
 * @see close()
 */
void NetManager::stopClient(Protocol protocol) {
  if (statusCheck(NET_CLIENT)) {
    std::cout << "NetManager: You're not a client, thus you can't be stopped." << std::endl;
    return;
  }

  if (protocol & PROTOCOL_TCP) {
    TCPsocket server = tcpSockets[netServer.tcpSocketIdx];
    unwatchSocket(&server);
    closeTCP(server);
    tcpSockets.pop_back();
    netServer.protocols ^= PROTOCOL_TCP;
  }
  if (protocol & PROTOCOL_UDP) {
    UDPsocket server = udpSockets[netServer.udpSocketIdx];
    unwatchSocket(&server);
    unbindUDPSocket(server, netServer.udpChannel);
    closeUDP(server);
    udpSockets.pop_back();
    netServer.protocols ^= PROTOCOL_UDP;
  }

  if (!netServer.protocols)
    resetManager();
}

/**
 * @brief Terminates all running servers or clients on all protocols.
 *
 * This function is called by the destructor, but it may be called explicitly
 * by the user if desired. It will call stopServer() or stopClient() as
 * appropriate.
 * @see stopServer()
 * @see stopClient()
 */
void NetManager::close() {
  if (netStatus & NET_SERVER) {
    stopServer(netServer.protocols);
  } else if (netStatus & NET_CLIENT) {
    stopClient(netServer.protocols);
  } else {
    std::cout << "NetManager: No active server or client. Nothing to do." << std::endl;
  }
}

/**
 * @brief Allows the dynamic addition of TCP or UDP to a running server or
 * client.
 *
 * Must be currently running as a server or client over only one of TCP or UDP.
 * This function adds and immediately launches the requested, missing protocol.
 * @param protocol TCP or UDP, given by PROTOCOL_XXX enum value.
 * @return True on success, false on failure.
 */
bool NetManager::addProtocol(Protocol protocol) {
  if (statusCheck(NET_SERVER, NET_CLIENT)) {
    std::cout << "NetManager: Not currently running a server or client." << std::endl;
    return false;
  }

  netProtocol |= protocol;
  netStatus |= NET_WAITING;

  return (netStatus & NET_SERVER) ? startServer() : startClient();
}

/**
 * @brief Set the protocol manually.
 *
 * This is currently useless as a public function given the structured use of
 * addNetworkInfo() and addProtocol(). It is only public because it seems like
 * it should be.
 * @param protocol TCP, UDP, or ALL; given by PROTOCOL_XXX enum value.
 */
void NetManager::setProtocol(Protocol protocol) {
  if (!statusCheck(NET_SERVER, NET_CLIENT)) {
    return;
  }

  netProtocol = protocol;
}

/**
 * @brief Set the port manually.
 *
 * Currently useless as a public function. This cannot be safely executed after
 * a server or client is launched.
 * @param port The desired port.
 */
void NetManager::setPort(Uint16 port) {
  if (!statusCheck(NET_SERVER, NET_CLIENT)) {
    return;
  }

  netPort = port;
}

/**
 * Set the host manually.
 *
 * Currently useless as a public function. This cannot be safely executed after
 * a server or client is launched.
 * @param host The desired host.
 */
void NetManager::setHost(const char *host) {
  if (!statusCheck(NET_SERVER, NET_CLIENT)) {
    return;
  }

  netHost = std::string(host);
}

/**
 * @brief Returns the currently active protocols.
 * @return The currently active protocols.
 */
Uint32 NetManager::getProtocol() {
  return (Uint32) netProtocol;
}

/**
 * @brief Returns the currently active port.
 * @return The currently active port.
 */
Uint16 NetManager::getPort() {
  return netPort;
}

/**
 * @brief Returns the currently active host.
 *
 * Must be running as a client to call this function; servers do not have hosts.
 * @return The currently active host.
 */
std::string NetManager::getHost() {
  if (statusCheck(NET_CLIENT)) {
    return std::string("Error: Invalid request.");
  }

  return netHost;
}




/* ****************************************************************************
 * Private
 */


bool NetManager::openServer(Protocol protocol, Uint16 port) {
  if (statusCheck(NET_WAITING))
    return false;

  if ((netStatus & NET_RESOLVED) && (netServer.protocols & protocol)) {
    std::cout << "NetManager: This protocol has already been established." << std::endl;
    return false;
  }

  if (netStatus & NET_CLIENT) {
    std::cout << "NetManager: Client already established. May not initiate server." << std::endl;
    return false;
  }

  if (SDLNet_ResolveHost(&netServer.address, NULL, port)) {
    std::cout << "SDL_net: Failed to start server!" << std::endl;
  } else {
    netServer.protocols |= protocol;
    netStatus ^= NET_WAITING;
    netStatus |= NET_RESOLVED;
  }

  return (protocol & PROTOCOL_TCP) ?
      openTCPSocket(&netServer.address) : openUDPSocket(port);
}

bool NetManager::openClient(Protocol protocol, std::string hostname, Uint16 port) {
  if (statusCheck(NET_WAITING))
    return false;

  if ((netStatus & NET_RESOLVED) && (netServer.protocols & protocol)) {
    std::cout << "NetManager: This protocol has already been established." << std::endl;
    return false;
  }

  if (netStatus & NET_SERVER) {
    std::cout << "NetManager: Server already established. May not initiate client." << std::endl;
    return false;
  }

  if (SDLNet_ResolveHost(&netServer.address, hostname.c_str(), port)) {
    std::cout << "SDL_net: Failed to resolve server!" << std::endl;
  } else {
    netServer.protocols |= protocol;
    netServer.hostname = hostname;
    netStatus ^= NET_WAITING;
    netStatus |= NET_RESOLVED;
  }

  return (protocol & PROTOCOL_TCP) ?
      openTCPSocket(&netServer.address) : openUDPSocket(port);
}

bool NetManager::openTCPSocket(IPaddress *addr) {
  bool ret = false;

  if (statusCheck(NET_RESOLVED))
    return false;

  TCPsocket tcpSock = SDLNet_TCP_Open(addr);

  if (!tcpSock)
    std::cout << "SDL_net: Failed to open TCP socket!" << std::endl;
  else {
    netServer.tcpSocketIdx = tcpSockets.size();
    netStatus |= NET_TCP_OPEN;
    tcpSockets.push_back(tcpSock);
    watchSocket(&tcpSock);
    ret = true;
  }

  return (netStatus & NET_SERVER) ? acceptTCP(tcpSock) : ret;
}

bool NetManager::openUDPSocket(Uint16 port) {
  bool ret = true;
  Uint16 udpPort = port;

  if (statusCheck(NET_RESOLVED))
    return false;

  if ((netStatus & NET_CLIENT) && forceClientRandomUDP)
    udpPort = PORT_RANDOM;

  UDPsocket udpSock = SDLNet_UDP_Open(udpPort);

  if (!udpSock) {
    std::cout << "SDL_net: Failed to open UDP socket!" << std::endl;
    ret = false;
  } else {
    udpSockets.push_back(udpSock);
    watchSocket(&udpSock);

    if (statusCheck(NET_UDP_OPEN)) {
      netServer.udpSocketIdx = udpSockets.size() - 1;
      netStatus |= NET_UDP_OPEN;

      if (netStatus & NET_CLIENT)
        return bindUDPSocket(udpSock, nextUDPChannel++, &netServer.address);
    }
  }

  return ret;
}

bool NetManager::acceptTCP(TCPsocket server) {
  bool ret = false;

  if (statusCheck(NET_SERVER | NET_TCP_OPEN))
    return false;

  TCPsocket tcpSock = SDLNet_TCP_Accept(server);

  if (!acceptNewClients || (tcpClients.size() >= SOCKET_TCP_MAX)) {
    if (!acceptNewClients)
      std::cout << "NetManager: TCP client rejected. Not accepting new clients." << std::endl;
    else
      std::cout << "NetManager: Exceeded max number of TCP connections." << std::endl;
    rejectTCPClient(tcpSock);
    return false;
  }

  if (!tcpSock)
    std::cout << "SDL_net: Failed to accept TCP client on server socket." << std::endl;
  else {
    ConnectionInfo *client = new ConnectionInfo;
    MessageBuffer *buffer = new MessageBuffer;
    IPaddress *addr = queryTCPAddress(tcpSock);
    client->hostname = SDLNet_ResolveIP(addr);
    client->protocols |= PROTOCOL_TCP;
    client->address.host = addr->host;
    client->address.port = addr->port;
    client->tcpSocketIdx = tcpSockets.size();
    client->tcpClientIdx = tcpClients.size();
    buffer->host = addr->host;
    buffer->updated = false;
    tcpClientData.push_back(buffer);
    tcpClients.push_back(client);
    tcpSockets.push_back(tcpSock);
    watchSocket(&tcpSock);

    netStatus |= NET_TCP_ACCEPT;
    ret = true;
  }

  return ret;
}

bool NetManager::bindUDPSocket (UDPsocket sock, int channel, IPaddress *addr) {
  bool ret = true;

  if (statusCheck(NET_UDP_OPEN))
    return false;

  int udpchannel;
  udpchannel = SDLNet_UDP_Bind(sock, channel, addr);

  if (udpchannel == -1) {
    std::cout << "SDL_net: Failed to bind UDP address to channel on socket."
        << std::endl;
    ret = false;
  } else if (statusCheck(NET_UDP_BOUND)) {
    netServer.udpChannel = udpchannel;
    netStatus |= NET_UDP_BOUND;
  } else {
    ConnectionInfo *client = new ConnectionInfo;
    MessageBuffer *buffer = new MessageBuffer;
    client->hostname = SDLNet_ResolveIP(addr);
    client->protocols |= PROTOCOL_UDP;
    client->address.host = addr->host;
    client->address.port = addr->port;
    client->udpSocketIdx = udpSockets.size() - 1;
    client->udpClientIdx = udpClients.size();
    client->udpChannel = udpchannel;
    buffer->host = addr->host;
    buffer->updated = false;
    udpClientData.push_back(buffer);
    udpClients.push_back(client);
  }

  return ret;
}

void NetManager::unbindUDPSocket(UDPsocket sock, int channel) {
  SDLNet_UDP_Unbind(sock, channel);
}

bool NetManager::sendTCP(TCPsocket sock, const void *data, int len) {
  bool ret = true;

  if (statusCheck(NET_TCP_ACCEPT, (NET_CLIENT | NET_TCP_OPEN)))
    return false;

  if (len > SDLNet_TCP_Send(sock, data, len)) {
    std::cout << "SDL_net: Failed to send TCP data." << std::endl;
    std::cout << SDLNet_GetError() << std::endl;
    ret = false;
  }

  return ret;
}

bool NetManager::sendUDP(UDPsocket sock, int channel, UDPpacket *pack) {
  bool ret = true;

  if (statusCheck(NET_UDP_BOUND) || !pack)
    return false;

  if (!SDLNet_UDP_Send(sock, channel, pack)) {
    std::cout << "SDL_net: Failed to send UDP data." << std::endl;
    std::cout << SDLNet_GetError() << std::endl;
    ret = false;
  }

  return ret;
}

bool NetManager::recvTCP(TCPsocket sock, void *data, int maxlen) {
  bool ret = true;

  if (statusCheck(NET_TCP_ACCEPT, (NET_CLIENT | NET_TCP_OPEN)))
    return false;

  if (0 >= SDLNet_TCP_Recv(sock, data, maxlen)) {
    std::cout << "SDL_net: Failed to receive TCP data." << std::endl;
    std::cout << SDLNet_GetError() << std::endl;
    ret = false;
  }

  return ret;
}

bool NetManager::recvUDP(UDPsocket sock, UDPpacket *pack) {
  bool ret = true;
  int result;

  if (statusCheck(NET_UDP_BOUND))
    return false;

  result  = SDLNet_UDP_Recv(sock, pack);

  if (result < 1) {
    ret = false;

    if (result < 0) {
      std::cout << "SDL_net: Failed to receive UDP data." << std::endl;
      std::cout << SDLNet_GetError() << std::endl;
    } else
      std::cout << "NetManager: No packets received." << std::endl;
  }

  return ret;
}

bool NetManager::sendUDPV(UDPsocket sock, UDPpacket **packetV, int npackets) {
  bool ret = true;

  if (statusCheck(NET_UDP_BOUND))
    return false;

  if (!SDLNet_UDP_SendV(sock, packetV, npackets)) {
    std::cout << "SDL_net: Failed to send UDP data." << std::endl;
    std::cout << SDLNet_GetError() << std::endl;
    ret = false;
  }

  return ret;
}

bool NetManager::recvUDPV(UDPsocket sock, UDPpacket **packetV) {
  bool ret = true;
  int result;

  if (statusCheck(NET_UDP_BOUND))
    return false;

  result  = SDLNet_UDP_RecvV(sock, packetV);

  if (result < 1) {
    ret = false;

    if (result < 0) {
      std::cout << "SDL_net: Failed to receive UDP data." << std::endl;
      std::cout << SDLNet_GetError() << std::endl;
    } else
      std::cout << "NetManager: No packets received." << std::endl;
  }

  return ret;
}

void NetManager::closeTCP(TCPsocket sock) {
  SDLNet_TCP_Close(sock);
  clearFlags(NET_TCP_ACCEPT | NET_TCP_OPEN);
  netServer.protocols ^= PROTOCOL_TCP;

  if (!netServer.protocols)
    close();
}

void NetManager::closeUDP(UDPsocket sock) {
  SDLNet_UDP_Close(sock);
  clearFlags(NET_UDP_BOUND | NET_UDP_OPEN);
  netServer.protocols ^= PROTOCOL_UDP;

  if (!netServer.protocols)
    close();
}

IPaddress* NetManager::queryTCPAddress(TCPsocket sock) {
  IPaddress *remote;

  remote = SDLNet_TCP_GetPeerAddress(sock);

  if (!remote) {
    std::cout << "SDL_net: Error retrieving remote TCP IP/port."
        "  This may be a server socket." << std::endl;
    std::cout << SDLNet_GetError() << std::endl;
  }

  return remote;
}

IPaddress* NetManager::queryUDPAddress(UDPsocket sock, int channel) {
  IPaddress *remote;

  remote = SDLNet_UDP_GetPeerAddress(sock, channel);

  if (!remote) {
    std::cout << "SDL_net: Error retrieving remote UDP IP address." << std::endl;
    std::cout << SDLNet_GetError() << std::endl;
  }

  return remote;
}

UDPpacket* NetManager::allocUDPpacket(int size) {
  UDPpacket *newPacket;

  newPacket = SDLNet_AllocPacket(size);

  if (!newPacket) {
    std::cout << "SDL_net: Unable to allocate UDP packet." << std::endl;
  }

  return newPacket;
}

bool NetManager::resizeUDPpacket(UDPpacket *pack, int size) {
  bool ret = true;
  int newSize;

  newSize = SDLNet_ResizePacket(pack, size);

  if (newSize < size) {
    std::cout << "SDL_net: Unable to resize UDP packet as requested." << std::endl;
    std::cout << SDLNet_GetError() << std::endl;
    ret = false;
  }

  return ret;
}

void NetManager::freeUDPpacket(UDPpacket **pack) {
  SDLNet_FreePacket(*pack);
  *pack = NULL;
}

void NetManager::watchSocket(TCPsocket *sock) {
  if (-1 == SDLNet_TCP_AddSocket(socketNursery, sock))
    std::cout << "SDL_net: Unable to add socket to SocketSet." << std::endl;
}

void NetManager::watchSocket(UDPsocket *sock) {
  if (-1 == SDLNet_UDP_AddSocket(socketNursery, sock))
    std::cout << "SDL_net: Unable to add socket to SocketSet." << std::endl;
}

void NetManager::unwatchSocket(TCPsocket *sock) {
  if (-1 == SDLNet_TCP_DelSocket(socketNursery, sock))
    std::cout << "SDL_net: Unable to remove socket from SocketSet." << std::endl;
}

void NetManager::unwatchSocket(UDPsocket *sock) {
  if (-1 == SDLNet_UDP_DelSocket(socketNursery, sock))
    std::cout << "SDL_net: Unable to remove socket from SocketSet." << std::endl;
}

bool NetManager::checkSockets(Uint32 timeout_ms) {
  int nReadySockets;
  bool ret = false;

  nReadySockets = SDLNet_CheckSockets(socketNursery, timeout_ms);

  if (nReadySockets == -1) {
    std::cout << "SDL_net: System error in CheckSockets." << std::endl;
    std::cout << SDLNet_GetError() << std::endl;
  } else if (nReadySockets) {
    ret = true;
    int i = 0;

    if (netServer.protocols & PROTOCOL_TCP) {
      if (netStatus & NET_SERVER) {
        if (SDLNet_SocketReady(tcpSockets[netServer.tcpSocketIdx])) {
          acceptTCP(tcpSockets[netServer.tcpSocketIdx]);
          nReadySockets--;
        }
        for (i = 0; i < tcpClients.size() && nReadySockets; i++) {
          if (SDLNet_SocketReady(tcpSockets[tcpClients[i]->tcpSocketIdx])) {
            readTCPSocket(i);
            nReadySockets--;
          }
        }
      } else if (netStatus & NET_CLIENT) {
        if (SDLNet_SocketReady(tcpSockets[netServer.tcpSocketIdx])) {
          readTCPSocket(SOCKET_SELF);
          nReadySockets--;
        }
      }
    }
    if (netServer.protocols & PROTOCOL_UDP) {
      if (SDLNet_SocketReady(udpSockets[netServer.udpSocketIdx])) {
        readUDPSocket(SOCKET_SELF);
        nReadySockets--;
      }
      if (netStatus & NET_SERVER) {
        for (i = 0; i < udpClients.size() && nReadySockets; i++) {
          if (SDLNet_SocketReady(udpSockets[udpClients[i]->udpSocketIdx])) {
            readUDPSocket(i);
            nReadySockets--;
          }
        }
      }
    }
  }

  return ret;
}

void NetManager::readTCPSocket(int clientIdx) {
  bool result;
  int idxSocket;
  MessageBuffer *mBuf;

  if (clientIdx == SOCKET_SELF) {
    idxSocket = netServer.tcpSocketIdx;
    mBuf = (netStatus & NET_SERVER) ? tcpClientData[netServer.tcpClientIdx] : &serverData;
  } else {
    idxSocket = tcpClients[clientIdx]->tcpSocketIdx;
    mBuf = tcpClientData[clientIdx];
  }

  result = recvTCP(tcpSockets[idxSocket], mBuf->output,
      MESSAGE_LENGTH);

  if (!result) {
    std::cout << "NetManager: Failed to read TCP packet from tcpClient " << clientIdx
        << "." << std::endl;
  } else
    mBuf->updated = true;
}

void NetManager::readUDPSocket(int clientIdx) {
  UDPpacket *buf;
  bool result;
  int idxSocket;
  MessageBuffer *mBuf;

  if (clientIdx == SOCKET_SELF) {
    idxSocket = netServer.udpSocketIdx;
    mBuf = (netStatus & NET_SERVER) ? udpClientData[netServer.udpClientIdx] : &serverData;
  } else {
    idxSocket = udpClients[clientIdx]->udpSocketIdx;
    mBuf = udpClientData[clientIdx];
  }

  buf = allocUDPpacket(MESSAGE_LENGTH);

  result = recvUDP(udpSockets[idxSocket], buf);

  if (!result) {
    std::cout << "NetManager: Failed to read UDP packet from udpClient "
        << clientIdx << "." << std::endl;
  } else {
    if (buf->channel == -1) {
      if (netStatus & NET_CLIENT) {
        std::cout << "NetManager: Unrecognized packet source." << std::endl;
      } else if (!addUDPClient(buf)) {
        std::cout << "NetManager: Unable to add new UDP client." << std::endl;
      }
    } else {
      memcpy(mBuf->output, buf->data, buf->len);
      mBuf->updated = true;
    }
  }

  if (buf)
    freeUDPpacket(&buf);
}

bool NetManager::addUDPClient(UDPpacket *pack) {
  bool ret = true;
  int socketIdx;

  if (!acceptNewClients) {
    std::cout << "NetManager: UDP client rejected. Not accepting new clients." << std::endl;
    rejectUDPClient(pack);
    return false;
  }

  if (nextUDPChannel >= CHANNEL_MAX) {
    if (openUDPSocket(PORT_DEFAULT))
      nextUDPChannel = CHANNEL_DEFAULT;
    else {
      std::cout << "NetManager: Exceeded max number of UDP connections." << std::endl;
      rejectUDPClient(pack);
      return false;
    }
  } else
    bindUDPSocket(udpSockets.back(), nextUDPChannel++, &pack->address);

  return ret;
}

void NetManager::rejectTCPClient(TCPsocket sock) {
  std::string deny = "Server full; connection rejected.";
  sendTCP(sock, deny.c_str(), deny.length());

  closeTCP(sock);
}

void NetManager::rejectUDPClient(UDPpacket *pack) {
  UDPpacket *packet;

  std::string deny = "Server full; connection rejected.";
  packet = craftUDPpacket(deny.c_str(), deny.length());
  packet->address.host = pack->address.host;
  packet->address.port = pack->address.port;
  sendUDP(udpSockets[netServer.udpSocketIdx], -1, packet);

  freeUDPpacket(&pack);
  freeUDPpacket(&packet);
}

UDPpacket* NetManager::craftUDPpacket(const char *buf, int len) {
  UDPpacket *packet;
  int header;

  if (len > MESSAGE_LENGTH) {
    std::cout << "NetManager: Message length exceeds current maximum." << std::endl;
    return NULL;
  }

  packet = allocUDPpacket(MESSAGE_LENGTH);

  if (!packet)
    return NULL;

  packet->len = len;
  memcpy(packet->data, buf, len);

  return packet;
}

void NetManager::processPacketData(const char *data) {
  /* TODO Scan copied data to check for messages to NetManager.
   *
   * Establish clear signals for 'drop client' et al.  How much will
   * be handled by the OGRE application, and how much will be taken
   * care of internally (by NetManager)?
   *
   */

}

bool NetManager::statusCheck(int state) {
  bool ret = (state != (netStatus & state));

  if (ret)
    std::cout << "NetManager: Invalid state for command." << std::endl;

  return ret;
}

bool NetManager::statusCheck(int state1, int state2) {
  bool ret1 = (state1 != (netStatus & state1));
  bool ret2 = (state2 != (netStatus & state2));
  bool result = ret1 && ret2;

  if (result)
    std::cout << "NetManager: Invalid state for command." << std::endl;

  return result;
}

void NetManager::clearFlags(int state) {
  int mask = netStatus & state;
  netStatus ^= mask;
}

void NetManager::resetManager() {
  int i;

  for (i = tcpClientData.size(); i >= 0; i--) {
    delete tcpClientData[i];
    tcpClientData.pop_back();
  }
  for (i = udpClientData.size(); i >= 0; i--) {
    delete udpClientData[i];
    udpClientData.pop_back();
  }
  for (i = tcpClients.size(); i >= 0; i--) {
    delete tcpClients[i];
    tcpClients.pop_back();
  }
  for (i = udpClients.size(); i >= 0; i--) {
    delete udpClients[i];
    udpClients.pop_back();
  }

  forceClientRandomUDP = true;
  acceptNewClients = true;
  nextUDPChannel = 1;

  // TODO Save last instance of info?
  netStatus = NET_INITIALIZED;
  netPort = PORT_DEFAULT;
  netProtocol = PROTOCOL_ALL;
  netHost.clear();
}



