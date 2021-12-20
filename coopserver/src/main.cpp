//
//  main.cpp
//  coopserver
//
//  Created by Vincent Moscaritolo on 12/19/21.
//

#include <iostream>
 
#include "LogMgr.hpp"

#include "TCPServer.hpp"
#include "Telnet/TelnetServerConnection.hpp"
#include "REST/RESTServerConnection.hpp"
#include "ServerCmdQueue.hpp"
#include "Telnet/CmdLineRegistry.hpp"

 

int main(int argc, const char * argv[]) {

	int telnetPort = 2020 ; // pumphouse.getDB()->getTelnetPort();
	int restPort =  8080 ; // pumphouse.getDB()->getRESTPort();
	bool remoteTelnet = true ;// pumphouse.getDB()->getAllowRemoteTelnet();

	 
	// create the server command processor
	auto cmdQueue = new ServerCmdQueue(NULL);
//	registerServerNouns();
//	registerCommandsLineFunctions();
	
	
	TCPServer telnet_server(cmdQueue);
	telnet_server.begin(telnetPort, true, [=](){
		return new TelnetServerConnection();
	});

	TCPServer rest_server(cmdQueue);
	rest_server.begin(restPort, remoteTelnet, [=](){
		return new RESTServerConnection();
	});

	
	 
	// run the main loop.
	while(true) {
//
//		pumphouse.setActiveConnections( rest_server.hasActiveConnections()
//												|| telnet_server.hasActiveConnections());
 	sleep(2);
	}

	return 0;



}
