//
// weno-server.cpp
//

#include <trantor/net/TcpServer.h>
#include "Server.h"

/**
 *  @brief Starts the server
 *
 */


int main(int argc, char* argv[])
{
    weno::Server session;

    session.startServer();

    return 0;
}
