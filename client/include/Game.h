#pragma once
#include <string>
#include <map>
#include <vector>
#include "event.h"



class Game{


    private:
    std::map<std::string,std::string> general_game_updates;
    std::map<std::string,std::string> teamA_updates; 
    std::map<std::string,std::string> teamB_updates;
    std::vector<Event> events;

    public:
    Game(std::map<std::string,std::string> general_game_updates,std::map<std::string,std::string> teamA_updates,std::map<std::string,std::string> teamB_updates,std::vector<Event> events);

    Game(const Game & otherGame);

    Game& operator=(const Game &otherGame);

    std::map<std::string,std::string>& get_genGameUpdates();

    std::map<std::string,std::string>& get_teamAUpdates();

    std::map<std::string,std::string>&get_teamBUpdates();

    std::vector<Event>& getEvents();

    void addEvent(Event& e);


};