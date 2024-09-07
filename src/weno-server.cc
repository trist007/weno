#include <iostream>
#include <string>
#include <trantor/net/TcpServer.h>
#include <trantor/utils/Logger.h>
#include <trantor/net/EventLoop.h>
#include <trantor/net/EventLoopThread.h>
#include <trantor/net/EventLoopThreadPool.h>
#include <trantor/net/InetAddress.h>
#include "../include/Weno.h"

using namespace trantor;

/**
 *  @brief Starts the server
 * 
 */
 

int main()
{
  LOG_DEBUG << "test start";
  Logger::setLogLevel(Logger::kTrace);
  EventLoopThread loopThread;
  loopThread.run();
#if USE_IPV6
  InetAddress addr(8888, true, true);
#else
  InetAddress addr(8888);
#endif
  TcpServer server(loopThread.getLoop(), addr, "weno");
  server.setBeforeListenSockOptCallback([](int fd)
                                        { std::cout << "setBeforeListenSockOptCallback:" << fd << std::endl; });
  weno::Chat session("trist007");
  server.setAfterAcceptSockOptCallback([](int fd)
                                       { std::cout << "afterAcceptSockOptCallback:" << fd << std::endl; });
  server.setRecvMessageCallback(
      [](const TcpConnectionPtr &connectionPtr, MsgBuffer *buffer)
      {
        // LOG_DEBUG<<"recv callback!";
        std::cout << std::string(buffer->peek(), buffer->readableBytes());
        connectionPtr->send(buffer->peek(), buffer->readableBytes());
        buffer->retrieveAll();
        // connectionPtr->forceClose();
      });
  server.setConnectionCallback([&session](const TcpConnectionPtr &connPtr)
                               {
    if (connPtr->connected()) {
      //std::string user = session.getUser();
      LOG_DEBUG << "New connection from " << session.getUser();
    } else if (connPtr->disconnected()) {
      LOG_DEBUG << "connection disconnected";
    } });
  server.setIoLoopNum(3);
  server.start();
  loopThread.wait();

  return 0;
}
