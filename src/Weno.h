#include <trantor/utils/MsgBuffer.h>
#include <trantor/net/inner/TcpConnectionImpl.h>
#include <string>
#include <memory>

using namespace trantor;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

namespace weno
{

  class Server
  {
    public:
      /**
       * @brief launch a new Chat.
       * 
       * @param user The user name.
       */
        Server();
        std::string getUser();
        ~Server();

        void Authenticate(const TcpConnectionPtr &connectionPtr, trantor::MsgBuffer *buffer);
        bool isAuthenticated();
    private:
        std::string m_user;
        bool m_authenticated;
  };

  class Client
  {
    public:
      /**
       * @brief launch a new Chat.
       * 
       * @param user The user name.
       */
        Client();
        Client(const std::string user);
        std::string getUser();
        ~Client();
    private:
        std::string m_user;
  };
}