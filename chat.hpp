//
//  chat.hpp
//  server
//
//  Created by Дима Щипицын on 05/12/2022.
//

#ifndef chat_hpp
#define chat_hpp

#include "ProtocolParser.hpp"
#include "database.hpp"

#include <string>
#include <vector>
#include <map>
#include <sstream>

class Connection;

struct Session
{
	int id = -1;
	std::string userName;
	std::string sendData;
	bool authorized = false;
};


struct Chat
{
public:
	Chat();
	~Chat();

	static void acc(Connection* c, void* u);

private:
	static void rd(Connection* c, void* u);
	static void wr(Connection* c, void* u);
	static void cl(Connection* c, void* u);

	std::string handler(Session& session, const std::string& data);
	std::string login(Session& session, const std::vector<std::string>& args);

	int sessionId_ = 0;
	std::map<int, Session> sessions_;
	ProtocolParser parser_;
	Database database_;
};

#endif /* chat_hpp */
