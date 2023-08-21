#pragma once
#include "ConnectionHandler.h"
#include <iostream>
#include <boost/atomic.hpp>
#include <boost/asio.hpp>
#include "event.h"
#include <vector>
#include "Game.h"

// TODO: implement the STOMP protocol
class StompProtocol
{
private:

int subscriptionId;
int receiptId;     // Need to update constructor for subId,recId , map
ConnectionHandler& handler;
boost::atomic_bool *connected;
std::string username;

public:
    void run();

    StompProtocol(int subId , int rId ,ConnectionHandler& handler,boost::atomic_bool *connected,std::string username);

    StompProtocol(const StompProtocol & otherProtocol);

    StompProtocol &operator=(const StompProtocol &otherProtocol);

    virtual ~StompProtocol();

    std::string handleLine(std::string& input,std::map<std::string,int>& topicToSubIdMap);

    std::string handleJoin(std::string& subscribe,std::map<std::string,int>& topicToSubIdMap);

    std::string handleExit(std::string& unsubscribe,std::map<std::string,int>& topicToSubIdMap);

    void handleReport(std::string& send);

    void handleSummary(std::string& summary);

    std::string handleLogout();

    int getSubscriptionId();

	int getReceiptId();

	// void addSubscription(std::string& topic , int subId);

	// void removeSubscription();

    

};
