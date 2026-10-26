#include "Window.h"

int main(int argc, const char *argv[]) {
    const Window window{};

    while (true) {
        window.update();
        window.clear();
    }
}