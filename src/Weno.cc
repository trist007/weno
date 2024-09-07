#include <string>
#include <iostream>
#include "../include/Weno.h"

using namespace weno;

Chat::Chat() : m_user("")
{
  std::cout << "Default constructor was used\n";
}

Chat::Chat(const std::string user)
  : m_user(user)
{
  std::cout << "Creating chat for " << user << std::endl;
}


Chat::~Chat()
{
  std::cout << "Closing chat\n";
}