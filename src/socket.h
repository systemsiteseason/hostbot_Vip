/*

   Copyright [2010] [Josko Nikolic]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   CODE PORTED FROM THE ORIGINAL GHOST PROJECT: http://ghost.pwner.org/

 */

#ifndef AURA_SOCKET_H_
#define AURA_SOCKET_H_

#include "util.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

typedef int32_t SOCKET;

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

#define closesocket close

//extern int32_t GetLastError();

#ifndef INADDR_NONE
#define INADDR_NONE -1
#endif

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

//
// CSocket
//

class CSocket
{
protected:
  SOCKET             m_Socket;
  struct sockaddr_in m_SIN;
  bool               m_HasError;
  int                m_Error;

  CSocket();
  CSocket(SOCKET nSocket, struct sockaddr_in nSIN);

public:
  ~CSocket();

  std::string                 GetErrorString() const;
  inline std::vector<uint8_t> GetPort() const { return CreateByteArray(m_SIN.sin_port, false); }
  inline std::vector<uint8_t> GetIP() const { return CreateByteArray(static_cast<uint32_t>(m_SIN.sin_addr.s_addr), false); }
  inline std::string          GetIPString() const { return inet_ntoa(m_SIN.sin_addr); }
  inline int32_t              GetError() const { return m_Error; }
  inline bool                 HasError() const { return m_HasError; }

  void SetFD(fd_set* fd, fd_set* send_fd, int32_t* nfds);
  void Reset();
  void Allocate(int type);
};

//
// CTCPSocket
//

class CTCPSocket : public CSocket
{
protected:
  std::string m_RecvBuffer;
  std::string m_SendBuffer;
  uint32_t    m_LastRecv;
  bool        m_Connected;

public:
  CTCPSocket();
  CTCPSocket(SOCKET nSocket, struct sockaddr_in nSIN);
  ~CTCPSocket();

  inline std::string* GetBytes() { return &m_RecvBuffer; }
  inline uint32_t     GetLastRecv() const { return m_LastRecv; }
  inline bool         GetConnected() const { return m_Connected; }

  inline void PutBytes(const std::string& bytes) { m_SendBuffer += bytes; }
  inline void PutBytes(const std::vector<uint8_t>& bytes) { m_SendBuffer += std::string(begin(bytes), end(bytes)); }

  inline void ClearRecvBuffer() { m_RecvBuffer.clear(); }
  inline void SubstrRecvBuffer(uint32_t i) { m_RecvBuffer = m_RecvBuffer.substr(i); }
  inline void                           ClearSendBuffer() { m_SendBuffer.clear(); }

  void DoRecv(fd_set* fd);
  void DoSend(fd_set* send_fd);
  void Disconnect();

  void Reset();
};

//
// CTCPClient
//

class CTCPClient final : public CTCPSocket
{
protected:
  bool m_Connecting;

public:
  CTCPClient();
  ~CTCPClient();

  inline std::string* GetBytes() { return &m_RecvBuffer; }
  inline bool         GetConnected() const { return m_Connected; }
  inline bool         GetConnecting() const { return m_Connecting; }

  void        Reset();
  inline void PutBytes(const std::string& bytes) { m_SendBuffer += bytes; }
  inline void PutBytes(const std::vector<uint8_t>& bytes) { m_SendBuffer += std::string(begin(bytes), end(bytes)); }

  bool        CheckConnect();
  inline void ClearRecvBuffer() { m_RecvBuffer.clear(); }
  inline void SubstrRecvBuffer(uint32_t i) { m_RecvBuffer = m_RecvBuffer.substr(i); }
  inline void                           ClearSendBuffer() { m_SendBuffer.clear(); }
  void DoRecv(fd_set* fd);
  void DoSend(fd_set* send_fd);
  void Disconnect();
  void Connect(const std::string& localaddress, const std::string& address, uint16_t port);
};

//
// CTCPServer
//

class CTCPServer final : public CTCPSocket
{
public:
  CTCPServer();
  ~CTCPServer();

  bool Listen(const std::string& address, uint16_t port);
  CTCPSocket* Accept(fd_set* fd);
};

//
// CUDPSocket
//

class CUDPSocket final : public CSocket
{
protected:
  struct in_addr m_BroadcastTarget;

public:
  CUDPSocket();
  ~CUDPSocket();

  bool SendTo(struct sockaddr_in sin, const std::vector<uint8_t>& message);
  bool SendTo(const std::string& address, uint16_t port, const std::vector<uint8_t>& message);
  bool Broadcast(uint16_t port, const std::vector<uint8_t>& message);

  void Reset();
  void SetBroadcastTarget(const std::string& subnet);
  void SetDontRoute(bool dontRoute);
};

#endif // AURA_SOCKET_H_
