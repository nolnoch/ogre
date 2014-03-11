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



/* ****************************************************************************
 * Global Structures
 */
/**
 * Allows integer manipulations of flags, but signals to users that the value
 * should be chosen from the PROTOCOL_XXX enumerated values.
 */
typedef int Protocol;

/**
 * These should be used whenever type Protocol is requested.
 */
enum {
  PROTOCOL_TCP        = 1024,                           //!< TCP bit flag.
  PROTOCOL_UDP        = 2048,                           //!< UDP bit flag.
  PROTOCOL_ALL        = PROTOCOL_TCP | PROTOCOL_UDP     //!< Combined bit flag.
};

/**
 * Internal state information packaging.
 */
struct ConnectionInfo {
  short tcpSocketIdx;                 //!< Index into the tcpSocket vector.
  short udpSocketIdx;                 //!< Index into the udpSocket vector.
  short tcpClientIdx;                 //!< Index into the tcpClients vector.
  short udpClientIdx;                 //!< Index into the udpClients vector.
  short tcpDataIdx;                   //!< Index into the tcpClientData vector.
  short udpDataIdx;                   //!< Index into the udpClientData vector.
  short udpChannel;                   //!< The associated UDP channel.
  Protocol protocols;                 //!< Associated protocols.
  IPaddress address;                  //!< This connection's IPaddress.
};

/**
 * External bins to which all received data is output and from which data may
 * be automatically pulled as input.
 * \b The \b user \b is \b responsible \b for \b clearing \b the \b updated
 * \b flag \b when \b data \b is \b retrieved!
 */
struct ClientData {
  Uint32 host;                        //!< To differentiate bin owners.
  bool updated;                       //!< Indicates new network output.
  char output[128];                   //!< Received network data.
  char input[128];                    //!< Target for automatic data pulls.
};



/* ****************************************************************************
 * Class
 */
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
 *
 * I've worked rather hard to eliminate dependency on Ogre3d-specific code so
 * that any application using SDL_net can plug this in and go.  I've done my
 * best to make it robust in that it supports simultaneous TCP and UDP and
 * makes use of some fairly automatic routines.
 *
 * Ideally, it should support more dynamic buffer sizing and the option to use
 * multiple ports.  The memory footprint doesn't seem to bad as of yet, but I'm
 * not done with it.
 *
 * Error and state checking was a priority in this implementation, so problems
 * of that sort should be minimal if not non-existent.  That said, If any code
 * errors are encountered, please fix them or contact me at the header address.
 */
class NetManager {
public:
  /* ***************************************************
   * Public
   */

  NetManager();
  virtual ~NetManager();

  /** @name Required Initialization Functions.                      *////@{
  bool initNetManager();
  void addNetworkInfo(Protocol protocol = PROTOCOL_ALL,
      Uint16 port = 0, const char *host = NULL);
  //! @}

  /** @name Control Functions.                                      *////@{
  bool startServer();
  bool startClient();
  bool scanForActivity();
  bool pollForActivity(Uint32 timeout_ms = 5000);
  void messageClients(const char *buf = NULL, int len = 0);
  void messageServer(const char *buf = NULL, int len = 0);
  void messageClient(Protocol protocol, int clientDataIdx, char *buf, int len);
  void dropClient(Protocol protocol, Uint32 host);
  void stopServer(Protocol protocol = PROTOCOL_ALL);
  void stopClient(Protocol protocol = PROTOCOL_ALL);
  void close();
  //! @}

  /** @name Getters & Setters.                                      *////@{
  bool addProtocol(Protocol protocol);
  void setProtocol(Protocol protocol);
  void setPort(Uint16 port);
  void setHost(const char *host);
  Uint32 getProtocol();
  Uint16 getPort();
  std::string getHost();
  //! @}

  ClientData tcpServerData;
  ClientData udpServerData;
  std::vector<ClientData *> tcpClientData;
  std::vector<ClientData *> udpClientData;


private:
  /* ***************************************************
   * Private
   */
  enum {
    ///@{
    /** State management flag bits. */
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
    ///@}
    ///@{
    /** Constants.                  */
    PORT_RANDOM         = 0,
    PORT_DEFAULT        = 51215,
    CHANNEL_AUTO        = -1,
    CHANNEL_DEFAULT     = 1,
    CHANNEL_MAX         = 2,    // Low for testing. Set to 6+ before launch.
    SOCKET_TCP_MAX      = 12,
    SOCKET_UDP_MAX      = 12,
    SOCKET_ALL_MAX      = SOCKET_TCP_MAX + SOCKET_UDP_MAX,
    SOCKET_SELF         = SOCKET_ALL_MAX + 1,
    MESSAGE_COUNT       = 10,
    MESSAGE_LENGTH      = 128
    ///@}
  };

  /** @name Direct SDL Call Wrappers                                *////@{
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
  int recvUDPV(UDPsocket sock, UDPpacket **packetV);
  void closeTCP(TCPsocket sock);
  void closeUDP(UDPsocket sock);
  IPaddress* queryTCPAddress(TCPsocket sock);
  IPaddress* queryUDPAddress(UDPsocket sock, int channel);
  //! @}

  /** @name  UDP Packet Management.                                  *////@{
  UDPpacket* craftUDPpacket(const char *buf, int len);
  UDPpacket* allocUDPpacket(int size);
  UDPpacket** allocUDPpacketV(int count, int size);
  bool resizeUDPpacket(UDPpacket *pack, int size);
  void freeUDPpacket(UDPpacket **pack);
  void freeUDPpacketV(UDPpacket ***pack);
  void processPacketData(const char *data);
  //! @}

  /** @name Socket Registration & Handling.                          *////@{
  void watchSocket(TCPsocket *sock);
  void watchSocket(UDPsocket *sock);
  void unwatchSocket(TCPsocket *sock);
  void unwatchSocket(UDPsocket *sock);
  bool checkSockets(Uint32 timeout_ms);
  void readTCPSocket(int clientIdx);
  void readUDPSocket(int clientIdx);
  //! @}

  /** @name Client Manipulation.                                     *////@{
  bool addUDPClient(UDPpacket *pack);
  void rejectTCPClient(TCPsocket sock);
  void rejectUDPClient(UDPpacket *pack);
  ConnectionInfo* lookupTCPClient(Uint32 host, bool create);
  ConnectionInfo* lookupUDPClient(Uint32 host, bool create);
  //! @}

  /** @name Helper Functions.                                        *////@{
  bool statusCheck(int state);
  bool statusCheck(int state1, int state2);
  void clearFlags(int state);
  void printError(std::string errorText);
  void resetManager();
  //! @}

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
