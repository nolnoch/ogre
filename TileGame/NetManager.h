/**
 * @file NetManager.h
 * @date March 7, 2014
 * @author Wade Burch (nolnoch@cs.utexas.edu)
 * @brief Networking wrapper for SDL_net created for OGRE engine use in CS 354R
 * at the University of Texas at Austin, taught by Don Fussell in Spring 2014.
 * @copyright GNU Public License
 */

#ifndef NETMANAGER_H_
#define NETMANAGER_H_


#include <vector>
#include <string>
#include <iostream>
#include <SDL/SDL_net.h>


/**
 * @typedef int Protocol
 * Makes it clearer to users that though an integer is being used, it should be
 * chosen from the PROTOCOL_XXX enumerated values.
 */
typedef int Protocol;

/**
 * @enum Protocol
 * These should be used whenever type Protocol is requested.
 */
enum {
  PROTOCOL_TCP        = 1024,                           //!< PROTOCOL_TCP
  PROTOCOL_UDP        = 2048,                           //!< PROTOCOL_UDP
  PROTOCOL_ALL        = PROTOCOL_TCP | PROTOCOL_UDP     //!< PROTOCOL_ALL
};

/**
 * @struct MessageBuffer
 * External bins to which all received data is output and from which data may
 * be automatically pulled as input.
 * @var host Objective, standardized identifier to differentiate bin owners.
 * @var updated True to indicate new network output before buffer is read.
 * @b The @b user @b is @b responsible @b for @b clearing @b this @b flag
 * @b when @b data @b is @b retrieved!
 * @var output Received network data to be handled by user.
 * @var input Target for automatic data pulls by NetManager on client updates.
 */
struct MessageBuffer {
  Uint32 host;
  bool updated;
  char output[128];
  char input[128];
};

/**
 * @class NetManager
 * @brief Networking wrapper for SDL_net for use in OGRE or similar engines.
 *
 * Currently allows simultaneous TCP/UDP connections on a single port.
 * While parameters may be given, the class is initialized to a default of
 * both TCP and UDP active on port 51215. Fully managed state preservation
 * prevents users from initiating illegal or undefined calls.  All retrieved
 * data is tunneled to public bins which must or may be checked by users.
 * Data to be sent may be specified or else is retrieved by default from the
 * established MessageBuffer bin.
 */
class NetManager {
public:
  NetManager();
  virtual ~NetManager();

  //! Required initialization functions. @{
  bool initNetManager();
  void addNetworkInfo(Protocol protocol = PROTOCOL_ALL,
      Uint16 port = 0, const char *host = NULL);
  //! @}

  //! Control functions. @{
  bool startServer();
  bool startClient();
  bool scanForActivity();
  bool pollForActivity(Uint32 timeout_ms = 5000);
  void messageClients(char *buf = NULL, int len = 0);
  void messageServer(char *buf = NULL, int len = 0);
  void messageClient(Protocol protocol, int clientDataIdx, char *buf, int len);
  void dropClient(Protocol protocol, int clientDataIdx);
  void stopServer(Protocol protocol = PROTOCOL_ALL);
  void stopClient(Protocol protocol = PROTOCOL_ALL);
  void close();
  //! @}

  /* Getters and setters. *////@{
  bool addProtocol(Protocol protocol);
  void setProtocol(Protocol protocol);
  void setPort(Uint16 port);
  void setHost(const char *host);
  Uint32 getProtocol();
  Uint16 getPort();
  std::string getHost();
  //! @}


  // Public data members.
  MessageBuffer serverData;
  std::vector<MessageBuffer *> tcpClientData;
  std::vector<MessageBuffer *> udpClientData;

private:
  enum {
    // State management flag bits.
    NET_UNINITIALIZED   = 0,
    NET_INITIALIZED     = 1,
    NET_WAITING         = 2,
    NET_RESOLVED        = 4,
    NET_TCP_OPEN        = 8,
    NET_UDP_OPEN        = 16,
    NET_TCP_ACCEPT      = 32,
    NET_UDP_BOUND       = 64,

    NET_SERVER          = 256,
    NET_CLIENT          = 512,

    // Constants.
    PORT_RANDOM         = 0,
    PORT_DEFAULT        = 51215,
    CHANNEL_AUTO        = -1,
    CHANNEL_DEFAULT     = 1,
    CHANNEL_MAX         = 2,    // Low for testing. Set to 6+ before launch.
    SOCKET_TCP_MAX      = 12,
    SOCKET_UDP_MAX      = 12,
    SOCKET_ALL_MAX      = SOCKET_TCP_MAX + SOCKET_UDP_MAX,
    SOCKET_SELF         = SOCKET_ALL_MAX + 1,
    MESSAGE_LENGTH      = 128
  };

  // Direct SDL call wrappers with state and error checking.
  bool openServer(Protocol protocol, Uint16 port);
  bool openClient(Protocol protocol, std::string addr, Uint16 port);
  bool openTCPSocket (IPaddress *addr);
  bool openUDPSocket (Uint16 port);
  bool acceptTCP(TCPsocket server);
  bool bindUDPSocket (UDPsocket sock, int channel, IPaddress *addr);
  void unbindUDPSocket(UDPsocket sock, int channel);
  bool sendTCP(TCPsocket sock, const void *data, int len);
  bool sendUDP(UDPsocket sock, int channel, UDPpacket *pack);
  bool recvTCP(TCPsocket sock, void *data, int maxlen);
  bool recvUDP(UDPsocket sock, UDPpacket *pack);
  bool sendUDPV(UDPsocket sock, UDPpacket **packetV, int npackets);
  bool recvUDPV(UDPsocket sock, UDPpacket **packetV);
  void closeTCP(TCPsocket sock);
  void closeUDP(UDPsocket sock);
  IPaddress* queryTCPAddress(TCPsocket sock);
  IPaddress* queryUDPAddress(UDPsocket sock, int channel);

  // UDP packet management.
  UDPpacket* craftUDPpacket(const char *buf, int len);
  UDPpacket* allocUDPpacket(int size);
  bool resizeUDPpacket(UDPpacket *pack, int size);
  void freeUDPpacket(UDPpacket **pack);
  void processPacketData(const char *data);

  // Socket registration and handling.
  void watchSocket(TCPsocket *sock);
  void watchSocket(UDPsocket *sock);
  void unwatchSocket(TCPsocket *sock);
  void unwatchSocket(UDPsocket *sock);
  bool checkSockets(Uint32 timeout_ms);
  void readTCPSocket(int clientIdx);
  void readUDPSocket(int clientIdx);

  // Client addition and rejection.
  bool addUDPClient(UDPpacket *pack);
  void rejectTCPClient(TCPsocket sock);
  void rejectUDPClient(UDPpacket *pack);

  // Helper functions.
  bool statusCheck(int state);
  bool statusCheck(int state1, int state2);
  void clearFlags(int state);
  void resetManager();

  // Internal state information packaging.
  struct ConnectionInfo {
    int tcpSocketIdx;
    int udpSocketIdx;
    int tcpClientIdx;
    int udpClientIdx;
    int udpChannel;
    Protocol protocols;
    IPaddress address;
    std::string hostname;
  };


  // Private data members.
  bool forceClientRandomUDP;
  bool acceptNewClients;
  int nextUDPChannel;
  int netStatus;
  int netPort;
  Protocol netProtocol;
  std::string netHost;
  ConnectionInfo netServer;
  std::vector<ConnectionInfo *> tcpClients;
  std::vector<ConnectionInfo *> udpClients;
  std::vector<TCPsocket> tcpSockets;
  std::vector<UDPsocket> udpSockets;
  SDLNet_SocketSet socketNursery;
};

#endif /* NETMANAGER_H_ */
