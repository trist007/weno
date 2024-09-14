#include <string>
#include <iostream>
#include "Weno.h"

using namespace weno;

//Server
Server::Server() : m_user(""), m_authenticated(false)
{
  std::cout << "Default constructor was used\n";
}

std::string Server::getUser()
{
  return m_user;
}

void Server::Authenticate(const TcpConnectionPtr &connectionPtr, trantor::MsgBuffer *buffer)
{
  connectionPtr->send("Please authenticate with a username: ");

  std::cout << "buffer->peek() = " << buffer->peek() << "\n";
  if (buffer->peek() == std::string("trist007"))
  {
    m_authenticated = true;
    std::cout << "user is authenticated" << std::endl; 
  }
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