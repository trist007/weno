//
// Server.h
//

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
        explicit Server();
        ~Server();
        void setUser(std::string);
        std::string getUser();
        void startServer();
        void userInput();

        //void Authenticate(const TcpConnectionPtr &connectionPtr, trantor::MsgBuffer *buffer);
        void Authenticate();
        bool isAuthenticated();
    private:
        std::string m_user;
        bool m_authenticated;
  };

}
