//
// Client.h
//

#include <trantor/utils/MsgBuffer.h>
#include <trantor/net/inner/TcpConnectionImpl.h>
#include <string>
#include <memory>

using namespace trantor;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

namespace weno
{

  class Client
  {
    public:
      /**
       * @brief launch a new Chat.
       * 
       * @param user The user name.
       */
        explicit Client();
        ~Client();
        void startClient();
        Client(const std::string user);
        void setUser(std::string);
        void joinUserInputThread();
        void startUserInput(const TcpConnectionPtr& conn);
        void userInput(const TcpConnectionPtr& conn);
        std::string getUser();
        std::string m_user;
    private:
        std::thread m_userInputThread;
        std::atomic<bool> m_stopThread{false};

  };
}