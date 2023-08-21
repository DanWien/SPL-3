#pragma once
#include <string>
#include <vector>
#include "ConnectionHandler.h"
#include <boost/atomic.hpp>
#include <boost/asio.hpp>
#include <thread>
#include "event.h"

class ServerListener {
    public:
    ServerListener(ConnectionHandler& handler , std::string userName , boost::atomic_bool *connected);

    ServerListener(const ServerListener&);

    ServerListener&operator=(const ServerListener&);

    virtual ~ServerListener();

    void run();




    private:
    ConnectionHandler& handler;
    std::string userName;
    boost::atomic_bool *connected;
    //Need to add the suggested object for keeping game events with who sent them
    void processMessage(std::string& message);

};