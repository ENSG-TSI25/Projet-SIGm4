#pragma once
#include <pqxx/pqxx>
#include "Config.hpp"
#include <memory>

class PostgresClient {
private:
    DBConfig config;
    std::unique_ptr<pqxx::connection> conn;

public:
    PostgresClient(const DBConfig& cfg);
    ~PostgresClient() = default;
    
    void connect();
    pqxx::result execute(const std::string& sql);
    bool isConnected() const;

    void initExtensions();
};