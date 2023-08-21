#include "Game.h"
#include <iostream>

Game::Game(std::map<std::string, std::string> general_game_updates, std::map<std::string, std::string> teamA_updates, std::map<std::string, std::string> teamB_updates, std::vector<Event> events):
general_game_updates(general_game_updates),teamA_updates(teamA_updates),teamB_updates(teamB_updates),events(events){}

Game::Game(const Game &otherGame):
general_game_updates(otherGame.general_game_updates),teamA_updates(otherGame.teamA_updates),teamB_updates(otherGame.teamB_updates),events(otherGame.events)
{
}

Game &Game::operator=(const Game &otherGame)
{
   general_game_updates=otherGame.general_game_updates;
   teamA_updates=otherGame.teamA_updates;
   teamB_updates=otherGame.teamB_updates;
   events=otherGame.events;
   return *this;
}

std::map<std::string, std::string> &Game::get_genGameUpdates()
{
    return general_game_updates;
}

std::map<std::string, std::string> &Game::get_teamAUpdates()
{
    return teamA_updates;
}

std::map<std::string, std::string> &Game::get_teamBUpdates()
{
    return teamB_updates;
}

std::vector<Event> &Game::getEvents()
{
    return events;
}

void Game::addEvent(Event& e)
{
    std::map<std::string,std::string> genUpdates=e.get_game_updates();
    for (std::map<std::string, std::string>::iterator it = genUpdates.begin(); it != genUpdates.end(); ++it) {
        general_game_updates[it->first] = it->second;
    }
    
    std::map<std::string,std::string> teamA=e.get_team_a_updates();
    for (std::map<std::string, std::string>::iterator it = teamA.begin(); it != teamA.end(); ++it) {
        teamA_updates[it->first] = it->second;
    }
    std::map<std::string,std::string> teamB=e.get_team_b_updates();
    for (std::map<std::string, std::string>::iterator it = teamB.begin(); it != teamB.end(); ++it) {
        teamB_updates[it->first] = it->second;
    }
    events.push_back(e);
}
