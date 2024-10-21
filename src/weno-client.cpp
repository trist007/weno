//
// weno-client.cpp
//

#include "Client.h"

#define USE_IPV6 0
int main()
{
    trantor::MsgBuffer buffer;
    weno::Client session;
    std::thread *c_pThreadUserInput  = nullptr;
    c_pThreadUserInput = new std::thread( [] ()
    {
      std::string userInput;
      while (true)
      {
         if (session.m_user.empty())
         {
             std::cout << "chat: ";
         }
         else
         {
            std::cout << session.m_user << ": ";
         }
         std::getline(std::cin, userInput);
         session.connSet_
         connectionPtr->send(buffer->peek(), buffer->readableBytes());
      }
    });
    session.startClient();
    return 0;
}
