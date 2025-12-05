#include <Application.hpp>
#include <iostream>
#include "core/proj_test.hpp"

int main() {
    try {
        proj_test();
        Application app;
        app.initialize();
        app.run();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}