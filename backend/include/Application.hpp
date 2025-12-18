#pragma once
#include <db/PostgresClient.hpp>
#include <memory>
#include <core/DataManager.hpp>

class Application {
private:
    std::unique_ptr<PostgresClient> dbClient;

public:
    Application();
    void initialize();
    void run();
};