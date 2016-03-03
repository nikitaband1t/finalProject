#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

static const char *found = "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n";
static const char *not_found = "HTTP/1.0 404 NOT FOUND\r\nContent-Type: text/html\r\n\r\n";
using boost::asio::ip::tcp;
static std::string directory;


class session {
private:
    tcp::socket socket_;
    enum {
        max_length = 1024
    };
    char data_[max_length];
public:
    session(boost::asio::io_service &io_service) : socket_(io_service) {
    }

    tcp::socket &scoket() { return socket_; }

    void start() {
        socket_.async_read_some(boost::asio::buffer(data_, max_length), boost::bind(&session::handle_read, this,
                                                                                    boost::asio::placeholders::error,
                                                                                    boost::asio::placeholders::bytes_transferred));
    };

    void handle_read(const boost::system::error_code &error,
                     size_t bytes_transferred) {
        if (!error) {
            std::string query(data_);
            std::string *file = new std::string("");
            file->append(directory.c_str()).append(strtok((strtok((char *) (query.c_str()), "GET ")),"?="));
            FILE*f = fopen("/home/box/output.txt","w");
            fwrite(file->c_str(),1,file->size(),f);
            fclose(f);
            FILE *file_ptr = fopen(file->c_str(), "r");
            if (file_ptr != nullptr) {
                boost::asio::write(socket_, boost::asio::buffer(found,strlen(found)));
                for (int bytes_read = fread(data_, sizeof(char), 1024, file_ptr); bytes_read != 0;
                    bytes_read = fread(data_, sizeof(char), 1024, file_ptr)) {
                    std::string deliver(data_);
                    boost::asio::write(socket_, boost::asio::buffer(deliver.c_str(),bytes_read));
                }
            } else {
                boost::asio::write(socket_, boost::asio::buffer(not_found,strlen(not_found)));
            }
            socket_.shutdown(boost::asio::socket_base::shutdown_type::shutdown_both);
            handle_write();
        } else {
            delete this;
        }
    }

    void handle_write() {
            socket_.async_read_some(boost::asio::buffer(data_, max_length),
                                    bind(&session::handle_read, this,
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));
    }
};

class server {
private:
    boost::asio::io_service &io_service_;
    tcp::acceptor acceptor_;
public:
    server(boost::asio::io_service &io_service, short port, std::string str) :
            io_service_(io_service),
            acceptor_(io_service, tcp::endpoint(boost::asio::ip::address::from_string(str.c_str()), port)) {
        session *new_session = new session(io_service_);
        acceptor_.async_accept(new_session->scoket(),
                               boost::bind(&server::handle_accept, this, new_session,
                                           boost::asio::placeholders::error));
    }

    void handle_accept(session *new_session,
                       const boost::system::error_code error) {
        if (!error) {
            new_session->start();
            new_session = new session(io_service_);
            acceptor_.async_accept(new_session->scoket(),
                                   boost::bind(&server::handle_accept, this, new_session,
                                               boost::asio::placeholders::error));
        } else {
            delete new_session;
        }
    }
};

class ioService {
    boost::asio::io_service *io_service;
public:
    ioService() {
        io_service = new boost::asio::io_service();
    }

    void serverMake(int port, std::string address) {
        server serv(*io_service, port, address);
        io_service->run();
    }

private:
    void run_service() {
        io_service->run();
    }
};

int main(int argc, char **argv) {
    ioService io_service;
    std::string address, port;
    int i = 0;
    while ((getopt(argc, argv, "h:p:d:") != -1)) {
        i++;
        switch (i) {
            case (1):
                address = optarg;
            case (2):
                port = optarg;
            case (3):
                directory = optarg;
        }
    }
    pid_t pid, sid;
    pid = fork();
    if (pid > 0)
        exit(EXIT_SUCCESS);
    sid = setsid();
    if ((chdir("/")) < 0) {
        exit(EXIT_FAILURE);
    }
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    io_service.serverMake(atoi(port.c_str()), address);
}
