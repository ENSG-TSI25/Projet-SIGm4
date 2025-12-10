#include <Application.hpp>
#include <iostream>

/**
 * @file main.cpp
 * @brief Main entry point for backend application
 * 
 * Creates and runs the Application instance.
 * Handles exceptions and returns exit code.
 */

/**
 * @brief Main function
 * @return 0 on success, 1 on error
 */
int main() {
    try {
        Application app;
        app.initialize();
        app.run();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}