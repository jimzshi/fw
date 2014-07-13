/*
 * simproto.cpp
 *
 *  Created on: Jul 12, 2014
 *      Author: zks
 */

#include <iostream>
#include <thread>
#include <future>
#include <memory>
#include <cstdint>
#include <chrono>

#include "libzks/u8string.h"
#include "libzks/simconf.h"
#include "libzks/simlog.h"
#include "libzks/hash.h"
#include "test.pb.h"
#include "asio.hpp"

using namespace std;
using asio::ip::tcp;

zks::simconf g_conf;
zks::simlog g_logger;
std::mutex g_accept_mutex;

using hr_clock_t = std::chrono::high_resolution_clock;
using time_point = std::chrono::time_point<hr_clock_t>;

class SimSession: public std::enable_shared_from_this<SimSession> {
protected:
    time_point start_tp_;
    tcp::socket socket_;
    tcp::endpoint peer_;
    uint32_t req_len_;
    uint32_t res_len_;
    std::string req_;
    std::string res_;
    zks::u8string session_id_;

    void read_req_len() {
        auto self(shared_from_this());
        asio::async_read(socket_, asio::buffer(&req_len_, sizeof(req_len_)),
                [this, self](const asio::error_code& error, std::size_t length) {
            if (!error) {
                ZKS_DEBUG(g_logger, session_id_.c_str(), "req len: %d", req_len_);
                req_.resize(req_len_);
                read_req_body(req_len_);
            } else {
                if(length) {
                    ZKS_WARN(g_logger, session_id_.c_str(), "read request len failed(%d): %s. read in: %d",
                            error.value(), error.message().c_str(), length);
                }
            }
        });
    }

    void read_req_body(std::size_t length) {
        auto self(shared_from_this());
        asio::async_read(socket_, asio::buffer((void*)req_.data(), req_.size()),[this, self](const asio::error_code& error, std::size_t length) {
            auto self(shared_from_this());
            if (error) {
                ZKS_WARN(g_logger, session_id_.c_str(), "read request failed(%d): %s",
                        error.value(), error.message().c_str());
                return ;
            }
            ZKS_TRACE(g_logger, session_id_.c_str(), "request body: %s",
                    zks::as_hex((uint8_t*)req_.data(), req_.size()).c_str());
            simproto::test::Request request;
            if(!request.ParseFromString(req_)) {
                ZKS_ERROR(g_logger, session_id_.c_str(), "%s", "parse from request failed. invalid request.");
                return ;
            }
            ZKS_INFO(g_logger, session_id_.c_str(), "request: %s", request.ShortDebugString().c_str());

            simproto::test::Response response;
            request_handler(request, response);
            ZKS_INFO(g_logger, session_id_.c_str(), "response: %s", response.ShortDebugString().c_str());

            if(!response.SerializePartialToString(&res_)) {
                ZKS_ERROR(g_logger, session_id_.c_str(), "%s", "serialize response failed.");
                return ;
            }
            res_len_ = res_.size();
            write_res_len();
        });
    }
    void request_handler(const simproto::test::Request& Req, simproto::test::Response& Res) {
        ZKS_INFO(g_logger, session_id_.c_str(), "%s", "start to handle request.");
        Res.set_ver("1.1");
        simproto::test::Response_ReturnCode* pCode = Res.mutable_code();
        pCode->set_err_no(13);
        pCode->set_err_msg("This is a error message for test");
        simproto::test::Response_ReturnBody* pBody = Res.add_body();
        pBody->set_type(simproto::test::Response::TEXT);
        pBody->set_content("body content for test response");
    }
    void write_res_len() {
        auto self(shared_from_this());
        asio::async_write(socket_, asio::buffer(&res_len_, sizeof(res_len_)),
                [this, self](std::error_code error, std::size_t length) {
                    if (!error) {
                        ZKS_DEBUG(g_logger, session_id_.c_str(), "write res len: %d", res_len_);
                        write_res_body();
                    } else {
                        ZKS_ERROR(g_logger, session_id_.c_str(), "write res len failed(%d): %s. wrote: %d",
                                error.value(), error.message().c_str(), length);
                    }
                }
        );
    }
    void write_res_body() {
        auto self(shared_from_this());
        asio::async_write(socket_, asio::buffer((void*)res_.data(), res_.size()),
                [this, self](std::error_code error, std::size_t length) {
                    if (!error) {
                        ZKS_DEBUG(g_logger, session_id_.c_str(), "write res body: %d", length);
                        read_req_len();
                    } else {
                        ZKS_ERROR(g_logger, session_id_.c_str(), "write res body failed(%d): %s. wrote: %d",
                                error.value(), error.message().c_str(), length);
                    }
                }
        );
    }
public:
    SimSession(tcp::socket socket, tcp::endpoint peer) :
            start_tp_(hr_clock_t::now()), socket_(std::move(socket)), peer_(std::move(peer)), req_len_(0), res_len_(0) {
        zks::HashCode32 h;
        asio::ip::address peer_addr = peer_.address();
        h += peer_addr.to_string().c_str();
        h += peer_.port();
        session_id_ = zks::to_u8string(h);
        ZKS_INFO(g_logger, session_id_.c_str(), "new session: [%s] from %s:%d started",
                session_id_.c_str(), peer_addr.to_string().c_str(), peer_.port());
    }
    void start() {
        read_req_len();
    }
    ~SimSession() {
        ZKS_INFO(g_logger, session_id_.c_str(), "session: [%s] finished. lasted for %dms", session_id_.c_str(),
                std::chrono::duration_cast<std::chrono::milliseconds>(hr_clock_t::now() - start_tp_).count());
    }
};

class SimServer {
    tcp::acceptor acceptor_;
    tcp::socket socket_;
    tcp::endpoint peer_;

    void accept_a_client() {
        acceptor_.async_accept(socket_, peer_, [&](const asio::error_code& error) {
            if(!error) {
                std::make_shared<SimSession>(std::move(socket_), std::move(peer_))->start();
            }
            accept_a_client();
        });
    }

public:
    SimServer(asio::io_service& io_service, short port) :
            acceptor_(io_service, tcp::endpoint(tcp::v4(), port)), socket_(io_service) {
        accept_a_client();
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage:\n\t" << argv[0] << " config-file\n";
        return -1;
    }
    if (g_conf.parse(argv[1], &std::cerr) < 0) {
        cerr << "Invalid config file!\n";
        return -2;
    }
    g_logger.configure(argv[1]);
    g_logger.reset();

    GOOGLE_PROTOBUF_VERIFY_VERSION;

    int thread_count = 0;
    if (g_conf.option_num("simproto", "thread_count", &thread_count) < 0) {
        thread_count = 20;
        ZKS_WARN(g_logger, "simproto-main", "thread_count is not found. use default setting: %d", thread_count);
    }
    ZKS_INFO(g_logger, "simproto-main", "use %d threads", thread_count);
    int port = 9999;
    if (g_conf.option_num("simproto", "port", &port) < 0) {
        port = 9999;
        ZKS_WARN(g_logger, "simproto-main", "port is not found. use default setting: %d", port);
    }
    ZKS_INFO(g_logger, "simproto-main", "use port number: %d", port);

    try{
        asio::io_service io_service;
        SimServer server(io_service, port);
        ZKS_NOTICE(g_logger, "simproto-main", "server started at: %d", port);
        io_service.run();
    } catch(std::exception& e) {
        ZKS_ERROR(g_logger, "simproto-main", "Caught exception: %s", e.what());
    }

    return 0;
}
