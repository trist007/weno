//
// Server.cpp
//

#include <trantor/net/TcpServer.h>
#include <trantor/utils/Logger.h>
#include <trantor/net/EventLoop.h>
#include <trantor/net/EventLoopThread.h>
#include <trantor/net/EventLoopThreadPool.h>
#include <trantor/net/InetAddress.h>
#include <trantor/net/TcpClient.h>

#include <string>
#include <iostream>
#include <atomic>
#ifdef _WIN32
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/tcp.h>
#endif

#include "Server.h"

#define USE_IPV6 0

using namespace trantor;
using namespace weno;

//Server
Server::Server() : m_user(""), m_authenticated(false)
{
  std::cout << "Default constructor was used\n";
}

void Server::setUser(std::string user)
{
  m_user = user;
}

std::string Server::getUser()
{
  return m_user;
}

std::thread* Server::userInput()
{
    MsgBuffer buffer;
    std::thread *s_pThreadUserInput  = nullptr;

    s_pThreadUserInput = new std::thread( [] (const TcpConnectionPtr& connectionPtr, MsgBuffer *buffer)
    {
      std::string userInput;
      while (true)
      {
         std::cout << "chat: ";
         std::getline(std::cin, userInput);
         connectionPtr->send(buffer->peek(), buffer->readableBytes());
      }
    });
    return s_pThreadUserInput;
}

void Server::startServer()
{
    LOG_DEBUG << "test start";
    Logger::setLogLevel(Logger::kTrace);
    auto userinput = userInput();
    EventLoopThread loopThread;
    loopThread.run();
#if USE_IPV6
    InetAddress addr(8888, true, true);
#else
    InetAddress addr(8888);
#endif
    TcpServer server(loopThread.getLoop(), addr, "test");
    server.setBeforeListenSockOptCallback([](int fd) {
        std::cout << "setBeforeListenSockOptCallback:" << fd << std::endl;
    });
    server.setAfterAcceptSockOptCallback([](int fd) {
        std::cout << "afterAcceptSockOptCallback:" << fd << std::endl;
    });
    server.setRecvMessageCallback(
        [](const TcpConnectionPtr &connectionPtr, MsgBuffer *buffer) {
            // LOG_DEBUG<<"recv callback!";
            std::cout << std::string(buffer->peek(), buffer->readableBytes());
            connectionPtr->send(buffer->peek(), buffer->readableBytes());
            buffer->retrieveAll();
            // connectionPtr->forceClose();
        });
    server.setConnectionCallback([&server](const TcpConnectionPtr &connPtr) {
        if (connPtr->connected())
        {
            LOG_DEBUG << "New connection";
            LOG_DEBUG << "printing connSet_";
            for ( auto element : server.connSet_)
            {
                std::cout << element << std::endl;
            }
        }
        else if (connPtr->disconnected())
        {
            LOG_DEBUG << "connection disconnected";
        }
    });
    server.setIoLoopNum(3);
    server.start();
    loopThread.wait();
}

//void Server::Authenticate(const TcpConnectionPtr &connectionPtr, trantor::MsgBuffer *buffer)
//{
//
//  connectionPtr->send("Please authenticate with a username: ");
//
//  std::string user = std::string(buffer->peek(), buffer->readableBytes());
//  buffer->retrieveAll();
//
//  if (user == std::string("trist007"))
//  {
//    m_authenticated = true;
//    Server::setUser(std::string(buffer->peek()));
//    std::cout << "user " << Server::getUser() << " is authenticated" << std::endl; 
//  }
//  else
//  {
//    LOG_DEBUG << "Authentication failed\n";
//  }
//}

void Server::Authenticate()
{
  m_authenticated = true;
}

bool Server::isAuthenticated()
{
  return m_authenticated;
}

Server::~Server()
{
  std::cout << "Closing chat\n";
}

