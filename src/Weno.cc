#include <string>
#include <iostream>
#include "../include/Weno.h"

using namespace weno;

//Server
Server::Server() : m_user("")
{
  std::cout << "Default constructor was used\n";
}

int Server::checkUser(std::string user)
{
  if (user == "trist007")
  {
    std::cout << "Authentication successful\n";
    return 0;
  }
  else
  {
    std::cout << "User does not exist\n";
    return 1;
  }
}

Server::~Server()
{
  std::cout << "Closing chat\n";
}

//Client
Client::Client() : m_user("")
{
  std::cout << "Default constructor was used\n";
}

Client::Client(std::string user)
  : m_user(user)
{
  m_user = user;
  std::cout << "Creating chat for " << user << std::endl;
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