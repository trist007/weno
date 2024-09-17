#include "Weno.h"

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
