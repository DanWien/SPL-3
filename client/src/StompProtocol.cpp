#include "StompProtocol.h"
#include <boost/thread.hpp>
#include <fstream>


StompProtocol::StompProtocol(int subId, int rId, ConnectionHandler &handler, boost::atomic_bool *connected,std::string username): 
subscriptionId(subId), receiptId(rId),handler(handler), connected(connected),username(username) {}



StompProtocol::StompProtocol(const StompProtocol & otherProtocol):
subscriptionId(otherProtocol.subscriptionId), receiptId(otherProtocol.receiptId), 
handler(otherProtocol.handler),connected(otherProtocol.connected),username(otherProtocol.username)
{}

StompProtocol &StompProtocol::operator=(const StompProtocol &otherProtocol)
{
    //handler = otherProtocol.handler; 
    subscriptionId=otherProtocol.subscriptionId;
    receiptId=otherProtocol.receiptId;
    connected = otherProtocol.connected;
    username = otherProtocol.username;
    return *this;
}

StompProtocol::~StompProtocol()
{
    connected=nullptr;
}

void StompProtocol::run()
{
    std::map<std::string,int> topicToSubIdMap;

    while(connected->load()){
        const short bufsize = 1024;
        char buf[bufsize];
        std::cin.getline(buf, bufsize);
        std::string line(buf);
        std::string frame=handleLine(line,topicToSubIdMap);
        if(frame!="")
          handler.sendFrameAscii(frame,'\0');
        if(line.find("logout")!=std::string::npos)
            connected->store(false);
        std::cin.clear();
        fflush(stdin);
    }
}



std::string StompProtocol::handleLine(std::string& input,std::map<std::string,int>& topicToSubIdMap)
{
    std::string output;
    int idx = input.find_first_of(" ");
    std::string command = input.substr(0,idx);
    input = input.substr(idx+1);
    if(command.compare("login") == 0){   
        std::cout<<"The client is already logged in, logout before trying again"<<std::endl;
        return "";
    }
    else if(command.compare("join") == 0)
        output = handleJoin(input,topicToSubIdMap);
    else if(command.compare("exit") == 0)
        output = handleExit(input, topicToSubIdMap);
    else if(command.compare("summary") == 0)     // Handle this case later
        handleSummary(input);
    else if(command.compare("report") == 0)
        handleReport(input);
    else if(command.compare("logout") == 0)
        output = handleLogout();
    return output;
}

std::string StompProtocol::handleJoin(std::string& subscribe,std::map<std::string,int>& topicToSubIdMap)
{
    int subId = getSubscriptionId();
    int rId = getReceiptId();
    std::string recId = std::to_string(rId);
    std::string subFrame = "SUBSCRIBE\ndestination:"+subscribe+'\n'+"id:" +std::to_string(subId)+ '\n'+"receipt:"+recId+'\n'+'\n';
    topicToSubIdMap[subscribe] = subId;
    std::string newCommand = "SUBSCRIBE " + subscribe;
    handler.addReceipt(recId, newCommand);
    std::map<std::string,std::map<std::string,Game>> map = handler.getMap();
    if(!(map.count(subscribe)>0)){
        std::map<std::string,Game> userGameMap;
        map[subscribe] = userGameMap;
    }
    return subFrame;
}

std::string StompProtocol::handleExit(std::string& unsubscribe,std::map<std::string,int>& topicToSubIdMap)
{
    if(topicToSubIdMap.count(unsubscribe) > 0) {  // Client is subscribed to this topic
        int subId = topicToSubIdMap[unsubscribe];
        int rId = getReceiptId();
        std::string recId = std::to_string(rId);
        std::string newCommand = "UNSUBSCRIBE " + unsubscribe;
        handler.addReceipt(recId,newCommand);
        std::string unSubFrame = "UNSUBSCRIBE\nid:"+std::to_string(subId)+'\n'+"receipt:"+recId+'\n'+'\n';
        //handler.getMap().erase(unsubscribe);
        return unSubFrame;
    }
    else {  // Client is not subscribed to this topic
        return "ERROR\nYou are not subscribed to this topic"; // Need to change this 
    }
}

void StompProtocol::handleReport(std::string &send)
{
    std::string jsonPath=std::string("data/")+send;     //might need to fix path
    names_and_events report=parseEventsFile(jsonPath);
    std::string teamA=report.team_a_name;
    std::string teamB=report.team_b_name;
    std::vector<Event> events=report.events;
    for(Event &event : events) {
        std::string sendFrame="SEND\ndestination:"+teamA+"_"+teamB+"\n\n"+"user: "+username+"\nteam a: "+teamA+"\nteam b: "+teamB+"\nevent name: "+event.get_name()+
                                "\ntime: "+std::to_string(event.get_time())+"\ngeneral game updates:\n";
        std::map<std::string,std::string>& generalUpdates = event.get_game_updates();
        std::map<std::string,std::string>& teamAUpdates = event.get_team_a_updates();
        std::map<std::string,std::string>& teamBUpdates = event.get_team_b_updates();
        for (std::map<std::string, std::string>::iterator it = generalUpdates.begin(); it != generalUpdates.end(); ++it) {
            std::string currUpdateName = it->first;
            std::string currUpdate = it->second;
            sendFrame = sendFrame + currUpdateName + ": " + currUpdate+"\n";
        }
        sendFrame = sendFrame+"team a updates:\n";

        for (std::map<std::string, std::string>::iterator it = teamAUpdates.begin(); it != teamAUpdates.end(); ++it) {
            std::string currUpdateName = it->first;
            std::string currUpdate = it->second;
            sendFrame = sendFrame + currUpdateName + ": " + currUpdate+"\n";
        }
        sendFrame = sendFrame+"team b updates:\n";

        for (std::map<std::string, std::string>::iterator it = teamBUpdates.begin(); it != teamBUpdates.end(); ++it) {
            std::string currUpdateName = it->first;
            std::string currUpdate = it->second;
            sendFrame = sendFrame + currUpdateName + ": " + currUpdate+"\n";
        }
        sendFrame = sendFrame+"description:\n"+event.get_discription();
        handler.sendFrameAscii(sendFrame,'\0');
    }
}
void StompProtocol::handleSummary(std::string& summary)
{
    int idx=summary.find_first_of(" ");
    std::string gameName= summary.substr(0,idx);
    summary=summary.substr(idx+1);
    idx=summary.find_first_of(" ");
    std::string user= summary.substr(0,idx);
    summary=summary.substr(idx+1);
    idx=summary.find_first_of(" ");
    std::string fileName= summary.substr(0,idx);
    std::map<std::string,Game>& userGameMap = handler.getMap()[gameName];
    Game& toSum=userGameMap.at(user);
    idx=gameName.find("_");
    std::string teamA=gameName.substr(0,idx);
    std::string teamB=gameName.substr(idx+1);
    std::string output=teamA +" vs "+teamB+"\nGame stats:\n";

    std::map<std::string,std::string>& stats=toSum.get_genGameUpdates();
    for (std::map<std::string, std::string>::iterator it = stats.begin(); it != stats.end(); ++it) {
        output=output+it->first + ": "+it->second+"\n";
    }
    output=output+teamA+" stats:\n";
    stats=toSum.get_teamAUpdates();
    for (std::map<std::string, std::string>::iterator it = stats.begin(); it != stats.end(); ++it) {
        output=output+it->first + ": "+it->second+"\n";
    }
    output=output+teamB+" stats:\n";
    stats=toSum.get_teamBUpdates();
    for (std::map<std::string, std::string>::iterator it = stats.begin(); it != stats.end(); ++it) {
        output=output+it->first + ": "+it->second+"\n";
    }

    output=output+"Game event reports:\n";
    std::vector<Event>& gameEvents=toSum.getEvents();
    for(Event& e:gameEvents){
        output=output+std::to_string(e.get_time())+" - "+e.get_name()+":\n\n"+e.get_discription()+"\n\n\n";
    }

    std::ofstream file;
    file.open(fileName,std::fstream::app);
    file<<output;
    file.close();
}


std::string StompProtocol::handleLogout()
{
    int receipt = getReceiptId();
    std::string recId = std::to_string(receipt);
    std::string newCommand = "DISCONNECT";
    handler.addReceipt(recId,newCommand);
    return "DISCONNECT\nreceipt:"+std::to_string(receipt)+"\n\n";
}


int StompProtocol::getSubscriptionId()
{
    return subscriptionId++;
}
	
int StompProtocol::getReceiptId()
{
    return receiptId++;
}


