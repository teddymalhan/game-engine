#include <raylib.h>
#include <raymath.h>

namespace {
    constexpr int kWindowWidth = 400;
    constexpr int kWindowHeight = 224;
    constexpr const char* kWindowTitle = "Template-4.0.0";

    void runGameLoop() {
        #pragma unroll
        while (!WindowShouldClose()) {
            BeginDrawing();
            ClearBackground(RAYWHITE);
            EndDrawing();
        }
    }
}

int main() {
    InitWindow(kWindowWidth, kWindowHeight, kWindowTitle);
    
    if (!IsWindowReady()) {
        return 1;
    }
    
    runGameLoop();
    
    CloseWindow();
    return 0;
}

