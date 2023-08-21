#include "StompProtocol.h"
#include <boost/thread.hpp>
#include <utility>
#include "ServerListener.h"

ServerListener::ServerListener(ConnectionHandler &handler, std::string userName, boost::atomic_bool *connected):
handler(handler),userName(std::move(userName)),connected(connected)

{}

ServerListener::ServerListener(const ServerListener & otherListener):
handler(otherListener.handler),userName(otherListener.userName),connected(otherListener.connected)
{}

ServerListener &ServerListener::operator=(const ServerListener &otherListener)
{
    //handler = otherListener.handler; 
    userName = otherListener.userName;
    connected = otherListener.connected;
    return *this;
}

ServerListener::~ServerListener()
{
    connected=nullptr;
}

void ServerListener::run()
{
    std::string ansFrame;
    while(connected->load()){
        ansFrame.clear();
        if(!handler.getFrameAscii(ansFrame,'\0')){
            std::cout << "Could not get a reply\n" << std::endl;
            break;
        }
        if(ansFrame.find("MESSAGE")!=std::string::npos)
            processMessage(ansFrame);
        else if(ansFrame.find("RECEIPT")!=std::string::npos){
            std::string header = "receipt-id:";
            int idx = ansFrame.find(header);
            std::string receiptId = ansFrame.substr(idx+header.length(),ansFrame.find("\n",idx)-idx-header.length());
            std::string command = handler.getCommand(receiptId);
            if(command.compare("No such command") == 0)
                std::cout << "No such receipt" << std::endl;
            else {
                if(command.find("DISCONNECT")!= std::string::npos) {
                    std::cout << "Disconnecting. Hope to see you again!" << std::endl;
                    connected->store(false);
                    break;
                }
                else if(command.find("UNSUBSCRIBE")!= std::string::npos) {
                    std::string channel = command.substr(command.find(" ")+1);
                    std::cout << "Exited channel "+channel << std::endl;
                }
                else if(command.find("SUBSCRIBE")!= std::string::npos) {
                    std::string channel = command.substr(command.find(" ")+1);
                    std::cout << "Joined channel "+channel << std::endl;
                }
            }     
        }
        else if(ansFrame.find("ERROR")!=std::string::npos){
            std::cout<<ansFrame+"\n\nDisconnected, try to log again"<<std::endl;
            connected->store(false);
            break;
        }
    }
}

void ServerListener::processMessage(std::string &message)
{
    message = message.substr(message.find("destination"));
    message=message.substr(message.find(":")+1);
    std::string destination= message.substr(0,message.find("\n"));
    message=message.substr(destination.length()+2);
    // Getting username
    int start = message.find("user:") + 6;
    int end = message.find_first_of("\n");
    std::string user = message.substr(start, end-start);
    // Getting teamA 
    start = message.find("team a:") + 8;
    end = message.find_first_of("\n", start);
    std::string teamA = message.substr(start, end-start);
    // Getting teamB
    start = message.find("team b:") + 8;
    end = message.find_first_of("\n", start);
    std::string teamB = message.substr(start, end-start);
    // Getting eventName
    start = message.find("event name:") + 12;
    end = message.find_first_of("\n", start);
    std::string eventName = message.substr(start, end-start);
    // Getting time
    start = message.find("time:") + 6;
    end = message.find_first_of("\n", start);
    std::string time = message.substr(start, end-start);
    // Getting generalUpdates
    start = message.find("general game updates:") + 22;
    end = message.find("team a updates:");
    std::string generalUpdates;
    if(start!=end)
        generalUpdates = message.substr(start, end-start-1);
    else
        generalUpdates = message.substr(start, end-start);
    // Getting teamAUpdates
    start = message.find("team a updates:") + 16;
    end = message.find("team b updates:");
    std::string teamAUpdates = message.substr(start, end-start);
    // Getting teamBUpdates
    start = message.find("team b updates:") + 16;
    end = message.find("description:");
    std::string teamBUpdates = message.substr(start, end-start);
    // Getting description
    start = message.find("description:") + 13;
    std::string description = message.substr(start);
    std::map<std::string,std::string> genUpdatesMap;
    while(generalUpdates.length()>1){
        std::string updateName=generalUpdates.substr(0,generalUpdates.find(":"));
        generalUpdates=generalUpdates.substr(updateName.length()+2);
        std::string updateValue=generalUpdates.substr(0,generalUpdates.find("\n"));
        generalUpdates=generalUpdates.substr(updateValue.length());
        while(updateName.at(0) == '\n')
            updateName = updateName.substr(1);
        genUpdatesMap[updateName]=updateValue;
    }

    std::map<std::string,std::string> teamAUpdatesMap;
    while(teamAUpdates.length()>1){
        std::string updateName=teamAUpdates.substr(0,teamAUpdates.find(":"));
        teamAUpdates=teamAUpdates.substr(updateName.length()+2);
        std::string updateValue=teamAUpdates.substr(0,teamAUpdates.find("\n"));
        teamAUpdates=teamAUpdates.substr(updateValue.length());
        while(updateName.at(0) == '\n')
            updateName = updateName.substr(1);
        teamAUpdatesMap[updateName]=updateValue;
    }

    std::map<std::string,std::string> teamBUpdatesMap;
    while(teamBUpdates.length()>1){
        std::string updateName=teamBUpdates.substr(0,teamBUpdates.find(":"));
        teamBUpdates=teamBUpdates.substr(updateName.length()+2);
        std::string updateValue=teamBUpdates.substr(0,teamBUpdates.find("\n"));
        teamBUpdates=teamBUpdates.substr(updateValue.length());
        while(updateName.at(0) == '\n')
            updateName = updateName.substr(1);
        teamBUpdatesMap[updateName]=updateValue;
    }

    Event reportedEvent(teamA,teamB,eventName,std::stoi(time),genUpdatesMap,teamAUpdatesMap,teamBUpdatesMap,description);
    std::map<std::string,std::map<std::string,Game>>& myMap = handler.getMap();
    std::string gameName = teamA+"_"+teamB;
    std::map<std::string,Game>& reportersMap = myMap[gameName];
    if(!(reportersMap.count(user) > 0)) {
        std::vector<Event> reportedEvents = {reportedEvent};
        Game g(genUpdatesMap,teamAUpdatesMap,teamBUpdatesMap,reportedEvents);
        reportersMap.emplace(user,g);
    }
    else {
        reportersMap.at(user).addEvent(reportedEvent);
    }
}

