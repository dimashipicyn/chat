//
//  database.hpp
//  server
//
//  Created by Дима Щипицын on 13/12/2022.
//

#ifndef database_hpp
#define database_hpp

#include <string>
#include <map>

struct UserData
{
	int id = 0;
	std::string name;
	std::string password;
};

enum {
	SUCCESS,
	NOCHANGED,
	ERROR
};

class Database
{
public:
	Database();
	~Database();

	int get_user_data(const std::string& name, UserData& userData);
	int update_user_data(const std::string& name, const UserData& userData);
	int remove_user_data(const std::string& name);

private:
	int userId_ = 1;
	std::map<std::string, UserData> users_;
};

#endif /* database_hpp */
