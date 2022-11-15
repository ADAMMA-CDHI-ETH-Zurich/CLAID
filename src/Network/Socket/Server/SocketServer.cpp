#include "Network/Socket/Server/SocketServer.hpp"

#ifdef _WIN32
    #include "SocketServerWindows_impl.hpp"
#else
    #include "SocketServerUnix_impl.hpp"
#endif