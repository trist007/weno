//
// Client.cpp
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
#include <thread>
#ifdef _WIN32
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/tcp.h>
#endif

#include "Client.h"

#define USE_IPV6 0

using namespace trantor;
using namespace weno;

Client::Client() : m_user("anonymous") {}

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
            [i, &loop, &connCount, &this](const TcpConnectionPtr &conn) {
                if (conn->connected())
                {
                    LOG_DEBUG << i << " connected!";
                    std::string tmp = std::to_string(i) + " client!!";
                    conn->send(tmp);
                    this->userInput();
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
            [](const TcpConnectionPtr &conn, MsgBuffer *buf) {
                LOG_DEBUG << std::string(buf->peek(), buf->readableBytes());
                buf->retrieveAll();
                //conn->shutdown();
            });
        client[i]->connect();
    }
    loop.loop();
}

void Client::joinUserInputThread()
{
    if (m_userInputThread.joinable())
    {
        m_userInputThread.join();
    }
}


void Client::startUserInput(const TcpConnectionPtr& conn)
{
    m_userInputThread = std::thread(&Client::userInput, this, conn);
}

void Client::userInput(const TcpConnectionPtr& conn)
{
    std::string userInput = std::string();
    while (!m_stopThread)
    {
        std::cout << this->m_user << ": ";

        try
        {
            if (!std::getline(std::cin, userInput))
            {
                std::cerr << "Error reading from input. Exiting...\n";
                break;
            }
        }
        catch (const std::exception& ex)
        {
            std::cerr << "Exception occurred while reading input: " << ex.what() << std::endl;
            break;
        }

        if (userInput == "/quit")
        {
            break;
        }
        else
        {
            try
            {
                conn->send(userInput);
            }
            catch (const std::exception& ex)
            {
                std::cerr << "Exception occurred while reading input: " << ex.what() << std::endl;
                break;
            }
        }
    }
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
    m_stopThread = true
    m_userInputThread.join();
    std::cout << "Closing chat\n";
}
