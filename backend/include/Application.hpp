#pragma once
#include <db/PostgresClient.hpp>
#include <memory>

class Application {
private:
    std::unique_ptr<PostgresClient> dbClient;

public:
    Application();
    void initialize();
    void run();
};