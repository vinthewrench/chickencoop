//
//  CoopMgrAPISecretMgr.cpp
//  coopserver
//
//  Created by Vincent Moscaritolo on 12/20/21.
//

#include "CoopMgrAPISecretMgr.hpp"
 #include "LogMgr.hpp"

CoopMgrAPISecretMgr::CoopMgrAPISecretMgr(CoopMgrDB* db){
	_db = db;
}

bool CoopMgrAPISecretMgr::apiSecretCreate(string APIkey, string APISecret){
	return _db->apiSecretCreate(APIkey,APISecret );
}

bool CoopMgrAPISecretMgr::apiSecretDelete(string APIkey){
	return _db->apiSecretDelete(APIkey);
}

bool CoopMgrAPISecretMgr::apiSecretGetSecret(string APIkey, string &APISecret){
	return _db->apiSecretGetSecret(APIkey, APISecret);
}

bool CoopMgrAPISecretMgr::apiSecretMustAuthenticate(){
	return _db->apiSecretMustAuthenticate();
}
