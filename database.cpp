//
//  database.cpp
//  server
//
//  Created by Дима Щипицын on 13/12/2022.
//

#include "database.hpp"

Database::Database()
{

}

Database::~Database()
{

}

int Database::get_user_data(const std::string& name, UserData& userData)
{
	if (auto user = users_.find(name); user != users_.end()) {
		userData = user->second;
		return SUCCESS;
	}
	return NOCHANGED;
}

int Database::update_user_data(const std::string& name, const UserData& userData)
{
	auto& user = users_[name];
	if (user.id == 0) {
		user.id = userId_++;
	}
	user.name = userData.name;
	user.password = userData.password;

	return SUCCESS;
}

int Database::remove_user_data(const std::string& name)
{
	users_.erase(name);
	return SUCCESS;
}
