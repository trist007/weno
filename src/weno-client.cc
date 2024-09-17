#include "Weno.h"

#define USE_IPV6 0
int main()
{
    weno::Client session;
    session.startClient();
    return 0;
}
