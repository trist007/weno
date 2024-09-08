#include <string>
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
        int checkUser(std::string user);
        ~Server();
    private:
        std::string m_user;
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