#include <string>
namespace weno
{

  class Chat
  {
    public:
      /**
       * @brief launch a new Chat.
       * 
       * @param user The user name.
       */
        Chat();
        Chat(const std::string user);
        ~Chat();
        std::string m_user;
  };
}