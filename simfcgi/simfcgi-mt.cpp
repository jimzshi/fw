/*
 * simfcgi-mt.cpp
 *
 *  Created on: Jul 4, 2014
 *      Author: zks
 */

#include "libzks/configure.h"
#include "libzks/json.h"
#include "libzks/u8string.h"
#include "libzks/utility.h"
#include "libzks/simconf.h"
#include "libzks/simlog.h"

#include "fcgi_config.h"

#ifdef ZKS_OS_GNULINUX_
#include <pthread.h>
#include <sys/types.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "fcgiapp.h"

#include <iostream>
#include <vector>
#include <thread>
#include <future>
#include <mutex>
#include <string>

using namespace std;

zks::simconf g_conf;
zks::simlog g_logger;
std::mutex g_accept_mutex;
int g_fcgi_sock;
zks::u8string fcgi_uri_path;

void fcgi_thread(int thread_id);
void fcgi_handler(const Json::Value& in, const zks::u8string& data, Json::Value* out);

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

    int thread_count = 0;
    if (g_conf.option_num("simfcgi", "thread_count", &thread_count) < 0) {
        thread_count = 20;
        ZKS_WARN(g_logger, "fcgi-main", "thread_count is not found. use default setting: %d", thread_count);
    }
    ZKS_INFO(g_logger, "fcgi-main", "use %d threads", thread_count);


    int port = -1;
    if (g_conf.option_num("simfcgi", "port", &port) < 0) {
        port = 9000;
        ZKS_WARN(g_logger, "fcgi-main", "thread_count is not found. use default setting: %d", thread_count);
    }
    ZKS_INFO(g_logger, "fcgi-main", "use port: %d", port);
    zks::u8string sock_port;
    sock_port.format(8, ":%d", port);


    int backlog = -1;
    if (g_conf.option_num("simfcgi", "backlog", &backlog) < 0) {
        backlog = 1024;
        ZKS_WARN(g_logger, "fcgi-main", "backlog is not found. use default setting: %d", backlog);
    }
    ZKS_INFO(g_logger, "fcgi-main", "use backlog: %d", backlog);

    if (g_conf.option("simfcgi", "fcgi_uri_path", &fcgi_uri_path) < 0) {
        fcgi_uri_path = "/fcgi/";
        ZKS_WARN(g_logger, "fcgi-main", "fcgi_uri_path is not found. use default setting: %s", fcgi_uri_path.c_str());
    }
    ZKS_INFO(g_logger, "fcgi-main", "use fcgi_uri_path: %s", fcgi_uri_path.c_str());

    FCGX_Init();
    g_fcgi_sock = FCGX_OpenSocket(sock_port.c_str(), backlog);
    if (g_fcgi_sock < 0) {
        ZKS_ERROR(g_logger, "fcgi-main", "%s", "can't create fcgi socket. quit now.");
        return -1;
    }

    std::vector<std::future<void>> futures;
    for (int i = 0; i < thread_count; ++i) {
        futures.push_back(std::async(std::launch::async, fcgi_thread, i));
    }
    ZKS_INFO(g_logger, "fcgi-main", "%d threads started.", thread_count);

    for (int i = 0; i < thread_count; ++i) {
        futures[i].get();
    }

    return 0;
}

void fcgi_thread(int thread_id) {
    zks::u8string thread_name;
    thread_name.format(32, "fcgi-%d", thread_id);
    const char* grp = thread_name.c_str();
    FCGX_Request request;
    int ret = FCGX_InitRequest(&request, g_fcgi_sock, 0);
    if (ret) {
        ZKS_ERROR(g_logger, grp, "failed to init request: %d. quit thread.", ret);
        return;
    }

    for (;;) {
        {
            std::lock_guard<std::mutex> Here(g_accept_mutex);
            int res = FCGX_Accept_r(&request);
            if (res < 0) {
                ZKS_ERROR(g_logger, grp, "failed to accept message: %d. ignore this session.", res);
                break;
            }
        }
        Json::Value in, out;
        in["fcgi_tid"] = thread_id;
        for (char** p = request.envp; *p; ++p) {
            string env(*p);
            ZKS_TRACE(g_logger, grp, "env: %s", env.c_str());
            auto pos = env.find_first_of('=');
            if (string::npos == pos) {
                ZKS_WARN(g_logger, grp, "invalid env: %s", *p);
            } else {
                ZKS_TRACE(g_logger, grp, "req: %s = %s", env.substr(0, pos).c_str(), env.substr(pos + 1).c_str());
                in["ENV"][env.substr(0, pos)] = env.substr(pos + 1);
            }
        }
        ZKS_INFO(g_logger, grp, "request-uri: %s?%s",
            in["ENV"]["SCRIPT_NAME"].isNull() ? "NULL" : in["ENV"]["REQUEST_URI"].asCString(),
            in["ENV"]["QUERY_STRING"].isNull() ? "NULL" : in["ENV"]["REQUEST_URI"].asCString());

        zks::u8string cmd = in["ENV"]["SCRIPT_NAME"].asString();
        cmd = cmd.trim_left(fcgi_uri_path);
        in["cmd"] = cmd.str();
        zks::u8string query = in["ENV"]["QUERY_STRING"].asString();
        std::vector<zks::u8string> params = query.split(true, "&", "");
        bool valid_params = true;
        for (auto const& p : params) {
            std::vector<zks::u8string> argv = p.split(true, "=", "");
            if (argv.size() == 0 || argv.size() > 2 || argv[0].size() == 0) {
                zks::u8string errmsg;
                errmsg.format(2048, "failed to parse param: %s. request denied.", p.c_str());
                (out)["retval"]["errno"] = 1;
                (out)["retval"]["errmsg"] = errmsg.str();
                ZKS_ERROR(g_logger, grp, "%s", errmsg.c_str());
                valid_params = false;
                break;
            }
            in["argv"][argv[0].str()] = argv.size() == 2 ? argv[1].str() : "";
        }


        zks::u8string data;
        if(in["ENV"]["REQUEST_METHOD"] == "POST") {
            const char* clen = in["ENV"]["CONTENT_LENGTH"].asCString();
            int len = atoi(clen);
            ZKS_INFO(g_logger, grp, "post data length: %d", len);
            data.resize(len, '\0');
            //char* input_buff = new char[len + 1];
            //int n = FCGX_GetStr(input_buff, len, request.in);
            int n = FCGX_GetStr((char*)data.data(), len, request.in);
            if(n < len) {
                ZKS_ERROR(g_logger, grp, "read in %d, less than %d. error.", n, len);
                out.clear();
                out["retval"]["errno"] = -1;
                out["retval"]["errmsg"] = "InputError";
                goto handler_finish;
            }
            //data.assign(input_buff, len);
            //delete[] input_buff;
        }
        if (valid_params) {
            fcgi_handler(in, data, &out);
        }

handler_finish:
        Json::FastWriter fwriter;
        string response = fwriter.write(out);
        ZKS_INFO(g_logger, grp, "response: %s", response.c_str());
        FCGX_FPrintF(request.out, "Content-type: text/html\r\n\r\n%s", response.c_str());

        FCGX_Finish_r(&request);
    }

    return;
}

void fcgi_handler(const Json::Value& in, const zks::u8string& data, Json::Value* out) {
    out->clear();
    (*out)["retval"]["errno"] = 0;
    (*out)["retval"]["errmsg"] = "OK";
    for (auto iter = in.begin(); iter != in.end(); ++iter) {
        (*out)["response"][iter.key().asString()] = *iter;
    }
    (*out)["data"] = data.str();
    return;
}
