//
//  server.cpp
//  2dCpp
//
//  Created by Dmitry Shipicyn on 28.11.2022.
//

#include "server.hpp"

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <string>
#include <memory>
#include <vector>

const int Connection::bufsz;

Connection::Connection(int id)
    : id_(id)
{
    
}

void Connection::bind(on_read_callback fr, on_write_callback fw, on_close_callback fc, void* user_data) {
    read_cb_ = fr;
    write_cb_ = fw;
    close_cb_ = fc;
    user_bind_data_ = user_data;
}

void Connection::send(const std::string& msg)
{
    int msg_size = std::min(static_cast<int>(msg.size()), bufsz);
    int ret = write(id_, msg.c_str(), msg_size);
    if (ret <= 0) {
        set_as_closed();
    }
}

void Connection::receive(std::string& msg)
{
    msg.clear();
    msg.resize(bufsz);
    int ret = read(id_, msg.data(), bufsz);
    if (ret > 0) {
        msg.resize(ret);
    }
    else {
        msg.clear();
        set_as_closed();
    }
}

void Connection::set_as_closed()
{
    must_close_ = true;
}

bool Connection::is_must_close()
{
    return must_close_;
}

int Connection::get_id()
{
    return id_;
}

void Connection::on_read() {
    if (read_cb_) {
        read_cb_(this, user_bind_data_);
    }
}

void Connection::on_write() {
    if (write_cb_) {
        write_cb_(this, user_bind_data_);
    }
}

void Connection::on_close() {
    if (close_cb_) {
        close_cb_(this, user_bind_data_);
    }
}

Server::Server()
{
    sockaddr_ = std::make_unique<sockaddr_in>();
    sockaddr_->sin_family = AF_INET;
    sockaddr_->sin_addr.s_addr = INADDR_ANY;
    sockaddr_->sin_port = htons(port_);
    
    if (listener_ = socket(AF_INET, SOCK_STREAM, 0); listener_ < 0) {
        printf("socket() failed\n");
        return;
    }
    
    int opt = 1;
    if (setsockopt(listener_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        printf("setsockopt() failed\n");
        return;
    }
    if (bind(listener_, (struct sockaddr *)sockaddr_.get(), sizeof(sockaddr_in)) < 0) {
        printf("bind() failed\n");
        return;
    }
    if (listen(listener_, 3) < 0) {
        printf("listen() failed\n");
        return;
    }
    
    is_initialized_ = true;
}

Server::~Server()
{
    
}

void Server::run()
{
    if (!is_initialized_) {
        printf("Server is not initialized!\n");
        return;
    }
    
    fd_set readfds;
    int max_fd = 0;
    
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    
    while (1) {
        FD_ZERO(&readfds);
        
        FD_SET(listener_, &readfds);
        max_fd = listener_;
        
        for (auto& conn : connections_) {
            FD_SET(conn.first, &readfds);
            max_fd = std::max(conn.first, max_fd);
        }
        
        if (select(max_fd + 1, &readfds, NULL, NULL, &tv) < 0) {
            continue;
        }
        accept(&readfds);
        read(&readfds);
        write();
    }
}

void Server::register_accept_func(on_accept_callback f, void* user_bind_data)
{
    accept_callback_ = f;
    user_bind_data_ = user_bind_data;
}

void Server::accept(fd_set* readfds) {
    socklen_t addrlen = 0;
    
    if (FD_ISSET(listener_, readfds)) {
        int new_sock = ::accept(listener_, (struct sockaddr *)sockaddr_.get(), &addrlen);
        if (new_sock > 0) {
            Connection new_conn(new_sock);
            if (accept_callback_) {
                accept_callback_(&new_conn, user_bind_data_);
            }
            connections_.emplace(new_sock, new_conn);
        }
    }
}

void Server::read(fd_set* readfds) {
    for (auto begin = connections_.begin(); begin != connections_.end();) {
        Connection& c = begin->second;
        int socket = begin->first;
        if (FD_ISSET(socket, readfds)) {
            c.on_read();
            if (c.is_must_close()) {
                c.on_close();
                close(socket);
                begin = connections_.erase(begin);
                continue;
            }
        }
        begin++;
    }
}

void Server::write() {
    for (auto begin = connections_.begin(); begin != connections_.end();) {
        Connection& c = begin->second;
        int socket = begin->first;
        
        c.on_write();
        if (c.is_must_close()) {
            c.on_close();
            close(socket);
            begin = connections_.erase(begin);
            continue;
        }
        begin++;
    }
}

struct User
{
    int id = 0;
    int room_id = 0;
    std::string name;
    
    std::vector<std::string> messages;
};

struct Room
{
    Room() {}
    Room(const std::string& n, int i)
    : id(i)
    , name(n)
    {
        
    }
    int id;
    std::string name;
    std::vector<int> users_ids;
};

struct Chat
{
    Chat()
    {
        rooms.emplace(1, Room("Room 1", 1));
        rooms.emplace(2, Room("Room 2", 2));
        rooms.emplace(3, Room("Room 3", 3));
    }
    static const int max_rooms = 10;
    static const int max_users_in_room = 3;
    
    Room& get_free_room() {
        for (auto& r : rooms) {
            if (r.second.users_ids.size() < max_users_in_room) {
                return r.second;
            }
        }
        int max_id = rooms.rbegin()->first;
        auto& r = rooms[max_id];
        r.id = max_id;
        r.name = "Room " + std::to_string(max_id);
        return r;
    }
    
    std::map<int, Room> rooms;
    std::map<int, User> users;
};

struct ctx
{
    Chat chat;
};

void rd(Connection* c, void* u)
{
    ctx* context = (ctx*)u;
    
    std::string msg;
    c->receive(msg);
    
    if (msg.empty()) {
        return;
    }
    
    User& user = context->chat.users[c->get_id()];
    Room& room = context->chat.rooms[user.room_id];

    for (auto user_id : room.users_ids) {
        if (user_id != user.id) {
            User& other_user = context->chat.users[user_id];
            other_user.messages.push_back(user.name + ": " + msg);
        }
    }
}

void wr(Connection* c, void* u)
{
    ctx* context = (ctx*)u;

    User& user = context->chat.users[c->get_id()];
    
    for (auto& msg : user.messages) {
        if (!msg.empty()) {
            c->send(msg);
        }
    }
    user.messages.clear();
}

void cl(Connection* c, void* u)
{
    ctx* context = (ctx*)u;
    
    User& user = context->chat.users[c->get_id()];
    Room& room = context->chat.rooms[user.room_id];
    
    room.users_ids.erase(std::remove(room.users_ids.begin(), room.users_ids.end(), user.id));
    context->chat.users.erase(user.id);
}

void acc(Connection* c, void* u)
{
    c->bind(rd, wr, cl, u);
    
    ctx* context = (ctx*)u;
    
    Room& room = context->chat.get_free_room();
    
    User user;
    user.id = c->get_id();
    user.name = "User " + std::to_string(c->get_id());
    user.room_id = room.id;
    
    room.users_ids.push_back(user.id);
    
    context->chat.users.emplace(c->get_id(), user);
}

int main()
{
    Server s;
    
    ctx c;
    
    s.register_accept_func(acc, &c);
    s.run();
    return 0;
}
