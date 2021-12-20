//
//  CoopMgrAPISecretMgr.hpp
//  coopserver
//
//  Created by Vincent Moscaritolo on 12/20/21.
//

#ifndef CoopMgrAPISecretMgr_hpp
#define CoopMgrAPISecretMgr_hpp
 
#include <stdio.h>

#include "ServerCmdQueue.hpp"
#include "CoopMgrDB.hpp"


class CoopMgrAPISecretMgr : public APISecretMgr {

public:
	CoopMgrAPISecretMgr(CoopMgrDB* db);
	
	virtual bool apiSecretCreate(string APIkey, string APISecret);
	virtual bool apiSecretDelete(string APIkey);
	virtual bool apiSecretGetSecret(string APIkey, string &APISecret);
	virtual bool apiSecretMustAuthenticate();
	
private:
	CoopMgrDB* 	 		_db;

};

#endif /* CoopMgrAPISecretMgr_hpp */
