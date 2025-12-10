#pragma once

#include <db/PostgresClient.hpp>
#include <memory>
#include <core/DataManager.hpp>

/**
 * @class Application
 * @brief Main backend application class
 * 
 * Initializes and coordinates all backend components.
 * This is the entry point for the backend system.
 */
class Application {
private:
    std::unique_ptr<PostgresClient> dbClient;  ///< Database client instance

public:
    /**
     * @brief Default constructor
     */
    Application();
    
    /**
     * @brief Initializes application components
     * 
     * Sets up database connection, data manager,
     * and other required services.
     */
    void initialize();
    
    /**
     * @brief Starts the application
     * 
     * Main loop or processing logic for the backend.
     * Called after initialization is complete.
     */
    void run();
};