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

#include "Weno.h"

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

void Server::startServer()
{
  LOG_DEBUG << "weno chat server starting...";
  Logger::setLogLevel(Logger::kTrace);
  EventLoopThread loopThread;
  loopThread.run();
#if USE_IPV6
  InetAddress addr(8888, true, true);
#else
  InetAddress addr(8888);
#endif
  TcpServer server(loopThread.getLoop(), addr, "weno");
  //auto policy = TLSPolicy::defaultServerPolicy("server.crt", "server.key");
  //server.enableSSL(std::move(policy));
  server.setBeforeListenSockOptCallback([](int fd)
                                        { std::cout << "setBeforeListenSockOptCallback:" << fd << std::endl; });

  server.setAfterAcceptSockOptCallback([](int fd)
                                       { std::cout << "afterAcceptSockOptCallback:" << fd << std::endl; });
  server.setRecvMessageCallback(
      [this](const TcpConnectionPtr &connectionPtr, MsgBuffer *buffer)
      {

        if (this->isAuthenticated() == false)
        {
          //session.Authenticate(connectionPtr, buffer);  


          connectionPtr->send("Please authenticate with a username: ");

          std::string user = std::string(buffer->peek(), buffer->readableBytes());
          buffer->retrieveAll();

          if (user == std::string("trist007"))
          {
            this->Authenticate();
            this->setUser(std::string(buffer->peek()));
            std::cout << "user " << this->getUser() << " is authenticated" << std::endl;
          }
          else
          {
            LOG_DEBUG << "Authentication failed\n";
          }
        }

        LOG_DEBUG << "recv callback!";
        std::cout << std::string(buffer->peek(), buffer->readableBytes());
        connectionPtr->send(buffer->peek(), buffer->readableBytes());
        buffer->retrieveAll();
        //connectionptr->forceclose();
        connectionPtr->send("You are authenticated");
      });

  server.setConnectionCallback([](const TcpConnectionPtr &connPtr)
                               {
    if (connPtr->connected()) {
      //LOG_DEBUG << "New connection from " << session.getUser();
      LOG_DEBUG << "New connection";
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

//Client
Client::Client() : m_user("") {}

void Client::setUser(std::string user)
{
  m_user = user;
}

void Client::startClient()
{
    trantor::Logger::setLogLevel(trantor::Logger::kTrace);
    LOG_DEBUG << "TcpClient class test!";
    EventLoop loop;
#if USE_IPV6
    InetAddress serverAddr("::1", 8888, true);
#else
    InetAddress serverAddr("127.0.0.1", 8888);
#endif
    std::shared_ptr<trantor::TcpClient> client[1];
    std::atomic_int connCount;
    connCount = 1;

    for (int i = 0; i < 1; ++i)
    {
        client[i] = std::make_shared<trantor::TcpClient>(&loop,
            serverAddr,
            "tcpclienttest");
        client[i]->setSockOptCallback([](int fd) {
            LOG_DEBUG << "setSockOptCallback!";
#ifdef _WIN32
#elif __linux__
            int optval = 10;
            ::setsockopt(fd,
                SOL_TCP,
                TCP_KEEPCNT,
                &optval,
                static_cast<socklen_t>(sizeof optval));
            ::setsockopt(fd,
                SOL_TCP,
                TCP_KEEPIDLE,
                &optval,
                static_cast<socklen_t>(sizeof optval));
            ::setsockopt(fd,
                SOL_TCP,
                TCP_KEEPINTVL,
                &optval,
                static_cast<socklen_t>(sizeof optval));
#else
#endif
            });
        client[i]->setConnectionCallback(
            [i, &loop, &connCount, this](const TcpConnectionPtr& conn) {
                if (conn->connected())
                {
                    LOG_DEBUG << i << " connected!";
                    std::string tmp = std::to_string(i) + " client!!";
                    //std::string user = session.getUser();
                    //conn->send(user);
                    std::string userInput;
                    std::cout << "Enter user input: ";
                    std::getline(std::cin, userInput);
                    conn->send(userInput);
                }
                else
                {
                    LOG_DEBUG << i << " disconnected";
                    --connCount;
                    if (connCount == 0)
                        loop.quit();
                }
            });
        client[i]->setMessageCallback(
            [](const TcpConnectionPtr& conn, MsgBuffer* buf) {
                LOG_DEBUG << std::string(buf->peek(), buf->readableBytes());
                buf->retrieveAll();
                std::string userInput;
                std::cout << "Enter user input: ";
                std::getline(std::cin, userInput);
                conn->send(userInput);

                //conn->shutdown();
            });
        client[i]->connect();
    }
    loop.loop();
}

std::string Client::getUser()
{
  if (m_user.empty())
  {
    return "No user";
  }
  else
  {
    return m_user;
  }
}

Client::~Client()
{
  std::cout << "Closing chat\n";
}