#include <db/PostgresClient.hpp>
#include <stdexcept>
#include <iostream>

PostgresClient::PostgresClient(const DBConfig& cfg)
    : config(cfg), conn(nullptr)
{}

void PostgresClient::connect() {
    std::string connStr =
        "host=" + config.host +
        " port=" + std::to_string(config.port) +
        " user=" + config.user +
        " password=" + config.password +
        " dbname=" + config.dbname;
    
    conn = std::make_unique<pqxx::connection>(connStr);
    
    if (!conn->is_open())
        throw std::runtime_error("PostgreSQL connection failed");
    
    std::cout << "Connected to database: " << conn->dbname() << std::endl;
}

pqxx::result PostgresClient::execute(const std::string& sql) {
    if (!conn || !conn->is_open())
        throw std::runtime_error("Connection not established");
    
    pqxx::work txn(*conn);
    pqxx::result r = txn.exec(sql);
    txn.commit();
    return r;
}

bool PostgresClient::isConnected() const {
    return conn && conn->is_open();
}

void PostgresClient::initExtensions() {
    if (!conn || !conn->is_open())
        throw std::runtime_error("Connection not established");
    
    pqxx::work txn(*conn);
    txn.exec("CREATE EXTENSION IF NOT EXISTS postgis;");
    txn.exec("CREATE EXTENSION IF NOT EXISTS postgis_topology;");
    txn.commit();
    
    std::cout << "PostGIS extensions initialized" << std::endl;
}