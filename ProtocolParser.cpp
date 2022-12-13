//
//  ProtocolParser.cpp
//  server
//
//  Created by Дима Щипицын on 13/12/2022.
//

#include "ProtocolParser.hpp"

#include <sstream>

ProtocolParser::ProtocolParser()
{
	commands_.insert(std::make_pair("LOGIN", &ProtocolParser::parseLogin));
}

ProtocolParser::~ProtocolParser()
{

}

Command ProtocolParser::parse(const std::string& data)
{
	std::stringstream ss(data);

	std::string cmdType;
	ss >> cmdType;
	if (auto foundCmd = commands_.find(cmdType); foundCmd != commands_.end()) {
		parseFunction f = foundCmd->second;
		return (this->*f)(ss);
	}

	Command command;

	return command;
}

Command ProtocolParser::parseLogin(std::stringstream& ss)
{
	Command command;
	command.type = LOGIN;

	std::string arg;
	ss >> arg;
	while (!arg.empty()) {
		command.args.push_back(std::move(arg));
		ss >> arg;
	}

	return command;
}
