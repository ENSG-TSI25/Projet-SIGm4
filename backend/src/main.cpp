#include <Application.hpp>
#include <iostream>

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