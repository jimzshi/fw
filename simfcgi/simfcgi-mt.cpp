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

void fcgi_thread(int thread_id);
void fcgi_handler(const Json::Value& in, Json::Value* out);

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

    FCGX_Init();
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
    FCGX_Request request;
    FCGX_InitRequest(&request, 0, 0);
    zks::u8string thread_name;
    thread_name.format(32, "fcgi-%d", thread_id);
    const char* grp = thread_name.c_str();

    for (;;) {
        {
            std::lock_guard<std::mutex> Here(g_accept_mutex);
            int res = FCGX_Accept_r(&request);
            if (res < 0) {
                ZKS_ERROR(g_logger, grp, "failed to accept message: %d. quit.", res);
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
                in[env.substr(0, pos)] = env.substr(pos + 1);
            }
        }
        ZKS_INFO(g_logger, grp, "request-uri: %s",
                in["REQUEST_URI"].isNull() ? "NULL" : in["REQUEST_URI"].asCString());

        fcgi_handler(in, &out);

        Json::FastWriter fwriter;
        string response = fwriter.write(out);
        ZKS_INFO(g_logger, grp, "response: %s", response.c_str());
        FCGX_FPrintF(request.out, "Content-type: text/html\r\n\r\n%s", response.c_str());

        FCGX_Finish_r(&request);
    }

    return;
}

void fcgi_handler(const Json::Value& in, Json::Value* out) {
    out->clear();
    (*out)["retval"]["errno"] = 0;
    (*out)["retval"]["errmsg"] = "OK";
    for (auto iter = in.begin(); iter != in.end(); ++iter) {
        (*out)["response"][iter.key().asString()] = *iter;
    }
    return;
}
