//
//  ProtocolParser.hpp
//  server
//
//  Created by Дима Щипицын on 13/12/2022.
//

#ifndef ProtocolParser_hpp
#define ProtocolParser_hpp

#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>

enum CommandType
{
	LOGIN,
	UNDEFINED
};

struct Command
{
	CommandType type = UNDEFINED;
	std::vector<std::string> args;
};

class ProtocolParser
{
public:
	ProtocolParser();
	~ProtocolParser();

	Command parse(const std::string& data);

private:
	using parseFunction = Command(ProtocolParser::*)(std::stringstream& ss);

	Command parseLogin(std::stringstream& ss);

	std::unordered_map<std::string, parseFunction> commands_;
};

#endif /* ProtocolParser_hpp */
