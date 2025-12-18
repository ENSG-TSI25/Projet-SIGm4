#include <db/Config.hpp>
#include <stdexcept>

DBConfig DBConfig::loadFromEnv() {
    DBConfig cfg;
    
    const char* host = std::getenv("DB_HOST");
    const char* port = std::getenv("DB_PORT");
    const char* user = std::getenv("DB_USER");
    const char* pass = std::getenv("DB_PASS");
    const char* name = std::getenv("DB_NAME");
    
    if (!host || !port || !user || !pass || !name)
        throw std::runtime_error("Missing database environment variables");
    
    cfg.host = host;
    cfg.port = std::stoi(port);
    cfg.user = user;
    cfg.password = pass;
    cfg.dbname = name;
    
    return cfg;
}