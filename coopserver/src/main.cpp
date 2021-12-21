//
//  main.cpp
//  coopserver
//
//  Created by Vincent Moscaritolo on 12/19/21.
//

#include <iostream>

#include "CommonIncludes.hpp"

#include "LogMgr.hpp"
#include "CoopMgr.hpp"
#include "CoopMgrAPISecretMgr.hpp"


#include "TCPServer.hpp"
#include "Telnet/TelnetServerConnection.hpp"
#include "REST/RESTServerConnection.hpp"
#include "ServerCmdQueue.hpp"
#include "Telnet/CmdLineRegistry.hpp"
#include "ServerCommands.hpp"
 

[[clang::no_destroy]]  CoopMgr	coopMgr;


int main(int argc, const char * argv[]) {

	LogMgr::shared()->_logFlags = LogMgr::LogLevelDebug;

	coopMgr.start();

	//set up the api secrets
	CoopMgrAPISecretMgr apiSecrets(coopMgr.getDB());

	auto db = coopMgr.getDB();

	uint16_t restPort = 	8080;
	uint16_t telnetPort = 2020;
	bool remoteTelnet = false;

 	db->getUint16Property(string(CoopMgrDB::PROP_TELNET_PORT),&telnetPort);
	db->getUint16Property(string(CoopMgrDB::PROP_REST_PORT),&restPort);
	db->getBoolProperty(string(CoopMgrDB::PROP_ALLOW_REMOTE_TELNET),&remoteTelnet);
 
	// create the server command processor
	auto cmdQueue = new ServerCmdQueue(&apiSecrets);
 	registerServerNouns();
 	registerCommandsLineFunctions();
	
	
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

		coopMgr.setActiveConnections( rest_server.hasActiveConnections()
												|| telnet_server.hasActiveConnections());
 	sleep(2);
	}

	return 0;



}
