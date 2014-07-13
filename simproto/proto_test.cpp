/*
 * proto_test.cpp
 *
 *  Created on: Jul 13, 2014
 *      Author: zks
 */

#include <iostream>
#include <thread>
#include <future>
#include <memory>
#include <cstdint>
#include <chrono>
#include <algorithm>
#include <numeric>

#include "libzks/u8string.h"
#include "libzks/simconf.h"
#include "libzks/simlog.h"
#include "libzks/hash.h"
#include "libzks/distributor.h"

#include "test.pb.h"
#include "asio.hpp"

using namespace std;
using asio::ip::tcp;

zks::simconf g_conf;
zks::simlog g_logger;
asio::io_service io_service;
tcp::endpoint svr_ept;

using hr_clock_t = std::chrono::high_resolution_clock;
using time_point = std::chrono::time_point<hr_clock_t>;


int send_a_request(int task_id) {
    zks::u8string task_str;
    task_str.format(32, "task-%d", task_id);
    ZKS_INFO(g_logger, task_str.c_str(), "task %d started", task_id);
    try{
        tcp::socket s(io_service);
        asio::error_code error;
        s.connect(svr_ept, error);
        if(!error) {
            ZKS_DEBUG(g_logger, task_str.c_str(), "%s", "connected");
        } else {
            ZKS_ERROR(g_logger, task_str.c_str(), "connect to %s:%d failed(%d): %s",
                    svr_ept.address().to_string().c_str(), svr_ept.port(), error.value(), error.message().c_str());
            return -1;
        }

        simproto::test::Request request;
        request.set_ver("1.4");
        request.set_cmd("echo");
        request.add_args("hello world!");
        std::string req = request.SerializeAsString();

        uint32_t req_len = req.size();
        uint32_t req_data_len = sizeof(req_len) + req_len;
        char* req_data = new char[req_data_len];
        std::memcpy(req_data, &req_len, sizeof(req_len));
        std::memcpy(req_data+sizeof(req_len), req.data(), req_len);
        ZKS_TRACE(g_logger, task_str.c_str(), "req data len: %d", req_data_len);
        ZKS_TRACE(g_logger, task_str.c_str(), "req data: %s", zks::as_hex((uint8_t*)req_data, req_data_len).c_str());
        asio::write(s, asio::buffer(req_data, req_data_len), error);
        if(!error) {
            ZKS_DEBUG(g_logger, task_str.c_str(), "%s is sent.", request.ShortDebugString().c_str());
            ZKS_TRACE(g_logger, task_str.c_str(), "%d(%d+%d) bytes are sent.", req_data_len, sizeof(req_len), req_len);
        } else {
            ZKS_ERROR(g_logger, task_str.c_str(), "sending to %s:%d failed(%d): %s",
                    svr_ept.address().to_string().c_str(), svr_ept.port(), error.value(), error.message().c_str());
            return -2;
        }
        delete[] req_data;

        uint32_t res_len;
        asio::read(s, asio::buffer(&res_len, sizeof(res_len)), error);
        if(!error) {
            ZKS_DEBUG(g_logger, task_str.c_str(), "response len: %d", res_len);
        } else {
            ZKS_ERROR(g_logger, task_str.c_str(), "reading response len failed(%d): %s", error.value(), error.message().c_str());
            return -3;
        }
        char* res_data = new char[res_len];
        uint32_t res_body_len = asio::read(s, asio::buffer(res_data, res_len), error);
        if(!error) {
            ZKS_DEBUG(g_logger, task_str.c_str(), "response body read in %d bytes.", res_body_len);
        } else {
            ZKS_ERROR(g_logger, task_str.c_str(), "reading response body failed(%d): %s. read in %d bytes.",
                    error.value(), error.message().c_str(), res_body_len);
            return -4;
        }
        simproto::test::Response response;
        if(!response.ParseFromArray(res_data, res_len)) {
            ZKS_ERROR(g_logger, task_str.c_str(), "%s", "can't parse this response body.");
            return -5;
        }
        ZKS_DEBUG(g_logger, task_str.c_str(), "response: %s", response.ShortDebugString().c_str());
        delete[] res_data;
    } catch(std::exception& e) {
        ZKS_ERROR(g_logger, task_str.c_str(), "Caught exception: %s", e.what());
    }
    ZKS_INFO(g_logger, task_str.c_str(), "task %d finished", task_id);
    return 0;
}

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
    if (g_conf.option_num("proto_test", "thread_count", &thread_count) < 0) {
        thread_count = 20;
        ZKS_WARN(g_logger, "proto_test-main", "thread_count is not found. use default setting: %d", thread_count);
    }
    ZKS_INFO(g_logger, "proto_test-main", "use %d threads", thread_count);
    unsigned short int server_port = 9999;
    if (g_conf.option_num("proto_test", "server_port", &server_port) < 0) {
        server_port = 9999;
        ZKS_WARN(g_logger, "proto_test-main", "server_port is not found. use default setting: %d", server_port);
    }
    ZKS_INFO(g_logger, "proto_test-main", "use server_port number: %d", server_port);
    int times = 5;
    if (g_conf.option_num("proto_test", "times", &times) < 0) {
        times = 5;
        ZKS_WARN(g_logger, "proto_test-main", "times is not found. use default setting: %d", times);
    }
    ZKS_INFO(g_logger, "proto_test-main", "test will push %d request on server", times);

    asio::ip::address_v4 localhost;
    localhost.from_string("127.0.0.1");
    svr_ept.address(localhost);
    svr_ept.port(server_port);

    std::vector<int> task_ids(times);
    std::iota(task_ids.begin(), task_ids.end(), 0);

    auto start_tp = hr_clock_t::now();
    std::vector<int> res = zks::for_each(task_ids.begin(), task_ids.end(), send_a_request, thread_count, std::chrono::milliseconds{5});
    auto end_tp = hr_clock_t::now();

    auto succ = std::count(res.begin(), res.end(), 0);
    ZKS_NOTICE(g_logger, "proto_test-main", "%d request sent to server at %d, using %d concurrent threads.",
            times, server_port, thread_count);
    ZKS_NOTICE(g_logger, "proto_test-main", "succeeded: %d, test lasted %dms, %f req/s", succ,
            std::chrono::duration_cast<std::chrono::milliseconds>(end_tp-start_tp).count(),
            float(times)/std::chrono::duration_cast<std::chrono::seconds>(end_tp-start_tp).count());
    return 0;
}



