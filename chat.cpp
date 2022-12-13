//
//  chat.cpp
//  server
//
//  Created by Дима Щипицын on 05/12/2022.
//

#include "chat.hpp"
#include "server.hpp"

#include <sstream>

static const int max_rooms = 10;
static const int max_users_in_room = 3;

Chat::Chat()
{
//	rooms.emplace(1, Room("Room 1", 1));
//	rooms.emplace(2, Room("Room 2", 2));
//	rooms.emplace(3, Room("Room 3", 3));
}

Chat::~Chat()
{

}

//Room& Chat::get_free_room()
//{
//	for (auto& r : rooms) {
//		if (r.second.users_ids.size() < max_users_in_room) {
//			return r.second;
//		}
//	}
//	int max_id = rooms.rbegin()->first;
//	auto& r = rooms[max_id];
//	r.id = max_id;
//	r.name = "Room " + std::to_string(max_id);
//	return r;
//}
//
//Room::Room()
//{
//
//}
//
//Room::Room(const std::string& n, int i)
//: id(i)
//, name(n)
//{
//
//}
//
//struct CommandUnknown : public Command
//{
//	void execute(User& user) {
//		User u;
//		u.name = "Server";
//		owner->send_msg(u, user.name, "Unknown command!");
//	}
//	Chat* owner;
//};
//
//Command* Chat::parse(const std::string& msg)
//{
//	std::stringstream ss(msg);
//
//	std::string cmd;
//	ss >> cmd;
//	if (cmd == "nick") {
//		return parse_nick(ss);
//	}
//	if (cmd == "msg") {
//		return parse_msg(ss);
//	}
//	auto c = new CommandUnknown;
//	c->owner = this;
//	return c;
//}
//
//struct CommandNick : public Command
//{
//	void execute(User& user) {
//		owner->add_nick(user, nick);
//	}
//	Chat* owner;
//	std::string nick;
//};
//
//struct CommandMsg : public Command
//{
//	void execute(User& user) {
//		owner->send_msg(user, nick, msg);
//	}
//	Chat* owner;
//	std::string nick;
//	std::string msg;
//};
//
//Command* Chat::parse_nick(std::stringstream& ss)
//{
//	std::string nick;
//	ss >> nick;
//	auto c = new CommandNick;
//	c->owner = this;
//	c->nick = nick;
//	return c;
//}
//
//Command* Chat::parse_msg(std::stringstream& ss)
//{
//	std::string nick;
//	std::string msg;
//	ss >> nick;
//	std::getline(ss, msg);
//
//	auto c = new CommandMsg;
//	c->owner = this;
//	c->nick = nick;
//	c->msg = msg;
//	return c;
//}
//
//void Chat::add_nick(User& user, const std::string nickname)
//{
//	if (!user.name.empty()) {
//		auto& dbUser = database[user.name];
//		if (database.find(nickname) == database.end()) {
//			dbUser.name = nickname;
//			user.name = nickname;
//		}
//	}
//	else {
//		if (database.find(nickname) == database.end()) {
//			auto& dbUser = database[nickname];
//			dbUser.name = nickname;
//			user.name = nickname;
//		}
//	}
//}
//
//void Chat::send_msg(User& user, const std::string nickname, const std::string& msg)
//{
//	auto& dbUser = database[nickname];
//	dbUser.messages.push_back(user.name + ":" + msg + "\n");
//}

std::string Chat::handler(Session& session, const std::string& data)
{
	Command command = parser_.parse(data);
	switch (command.type) {
		case LOGIN:
			return login(session, command.args);

		default:
			break;
	}
	return "<SERVER> unknown command\n";
}

std::string Chat::login(Session& session, const std::vector<std::string>& args)
{
	std::string login = args[0];
	std::string password = args[1];

	UserData userData;
	if (database_.get_user_data(login, userData) != SUCCESS) {
		return "<SERVER> unauthorized\n";
	}

	if (userData.password != password) {
		return "<SERVER> unauthorized\n";
	}

	session.userName = login;
	session.authorized = true;

	return "<SERVER> authorized";
}

void Chat::rd(Connection* c, void* u)
{
    Chat* chat = (Chat*)u;

    std::string data;
    c->receive(data);

	auto& session = chat->sessions_[c->get_id()];
	session.sendData = chat->handler(session, data);
}

void Chat::wr(Connection* c, void* u)
{
    Chat* chat = (Chat*)u;

	auto& session = chat->sessions_[c->get_id()];

	if (!session.sendData.empty()) {
		c->send(session.sendData);
		session.sendData.clear();
	}
}

void Chat::cl(Connection* c, void* u)
{
    Chat* chat = (Chat*)u;

	chat->sessions_.erase(c->get_id());
}

void Chat::acc(Connection* c, void* u)
{
    c->bind(rd, wr, cl, u);

    Chat* chat = (Chat*)u;

	auto& session = chat->sessions_[c->get_id()];
	session.id = chat->sessionId_++;
	
}
