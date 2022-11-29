//
//  server.hpp
//  2dCpp
//
//  Created by Dmitry Shipicyn on 28.11.2022.
//

#ifndef server_hpp
#define server_hpp

#include <map>
#include <memory>
#include <array>

struct sockaddr_in;
struct fd_set;
class Server;

class Connection {
public:
    Connection(int s);
    static const int bufsz = 1024;
    
    int sock;
    int receive_size;
    int send_size;
    char receive[bufsz];
    char send[bufsz];
    
    using on_read_callback = void (*)(Connection* conn, void* user_bind_data);
    using on_write_callback = void (*)(Connection* conn, void* user_bind_data);
    using on_close_callback = void (*)(Connection* conn, void* user_bind_data);
    
    void bind(on_read_callback fr, on_write_callback fw, on_close_callback fc, void* user_data);
    
    void on_read();
    void on_write();
    void on_close();

private:
    on_read_callback read_f = nullptr;
    on_write_callback write_f = nullptr;
    on_close_callback close_f = nullptr;
    void* user_bind_data = nullptr;
};

class Server
{
public:
    Server();
    ~Server();
    
    using on_accept_callback = void (*)(Connection* conn, void* user_bind_data);
    void register_accept_func(on_accept_callback f, void* user_bind_data);
    
    void run();
    
private:
    void accept(fd_set* readfds);
    void read(fd_set* readfds);
    void write();
    
    static const short port_ = 12700;
    
    std::unique_ptr<sockaddr_in> sockaddr_;
    
    on_accept_callback accept_callback_;
    void* user_bind_data_;
    
    int listener_;
    std::map<int, Connection> connections_;
    bool is_initialized_ = false;
};

#endif /* server_hpp */
