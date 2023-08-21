#include <iostream>
#include <string>
#include <boost/atomic.hpp>
#include "ConnectionHandler.h"
#include "StompProtocol.h"
#include "ServerListener.h"
#include <boost/thread.hpp>
#include <thread>
#include "Game.h"

int main(int argc, char *argv[])
{
	while(1){
		boost::atomic_bool *connected = new boost::atomic_bool(false);
		const short bufsize = 1024;
		char buf[bufsize];
		std::string login,host,username,password;
		short port;
		while (!connected->load())   // Need to change this : take care of cases where client is already logged in.
		{
			std::cin.getline(buf, bufsize); // Now we need to use the login command for host/port
			login = buf;
			login = login.substr(login.find_first_of(" ") + 1);
			int idx = login.find_first_of(":");
			host = login.substr(0, idx);
			login = login.substr(idx + 1);
			idx = login.find_first_of(" ");
			port = std::stoi(login.substr(0, idx));
			login = login.substr(idx + 1);
			idx = login.find_first_of(" ");
			username = login.substr(0, idx);
			password = login.substr(idx + 1);
			std::map<std::string,std::map<std::string,Game>> map;
			std::map<std::string,std::string> receiptMap;
			ConnectionHandler handler(host, port,map,receiptMap);
			if (!handler.connect())
			{
				std::cerr << "Could not connect to server" << host << ":" << port << std::endl;
				return 1;
			}
			else
			{
				*connected = true;
				std::string connectFrame = "CONNECT\naccept-version:1.2\nhost:stomp.cs.bgu.ac.il\nlogin:" + username + "\npasscode:" + password + "\n\n\0";
				handler.sendFrameAscii(connectFrame, '\0');
				std::string ansFrame;
				if (!handler.getFrameAscii(ansFrame, '\0'))
				{
					std::cout << "Connection was closed before receiving a reply\n"
							<< std::endl;
					break;
				}
				if(ansFrame.find("CONNECTED")!=std::string::npos) 
				{
					if(ansFrame.find("version:1.2"))
					{
						std::cout<<"Login successful" << std::endl;
						StompProtocol keyBoard(0,0,handler,connected,username);
						ServerListener sl(handler,username,connected);
						std::thread serverListenerThread(&ServerListener::run,&sl);
						keyBoard.run();
						serverListenerThread.join();
					}
					else 
						std::cout<<"Version is incompatibale or missing" << std::endl;
				}
				else if(ansFrame.find("ERROR")!=std::string::npos) {
					if(ansFrame.find("user is already logged in")!=std::string::npos)
						std::cout<<"User already logged in" << std::endl;
					else if(ansFrame.find("wrong password")!=std::string::npos)
						std::cout<<"Wrong password - The frame was :\n" + ansFrame << std::endl;
				}		
			}
		}
		delete connected;
	}
	return 0;
}