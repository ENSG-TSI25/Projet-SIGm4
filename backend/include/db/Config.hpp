#pragma once
#include <string>
#include <cstdlib>

struct DBConfig {
    std::string host;
    int port;
    std::string user;
    std::string password;
    std::string dbname;

    static DBConfig loadFromEnv();
};