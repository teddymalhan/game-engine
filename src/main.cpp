#include <raylib.h>
#include <raymath.h>
#include <project/scene_manager.hpp>
#include <project/tree_scene.hpp>
#include <project/geometric_scene.hpp>
#include <project/bullet_physics_scene.hpp>
#include <iostream>
#include <memory>

namespace {
    constexpr int kWindowWidth = 800;
    constexpr int kWindowHeight = 600;
    constexpr const char* kWindowTitle = "Raylib 3D Scene Example";
    constexpr const char* kModelPath = "assets/retrourban/tree-small.glb";
    
    constexpr float kCameraPositionX = 2.0F;
    constexpr float kCameraPositionY = 1.5F;
    constexpr float kCameraPositionZ = 2.0F;
    constexpr float kCameraFovy = 45.0F;
    constexpr int kTargetFPS = 60;
    constexpr int kGridSlices = 10;
    constexpr float kGridSpacing = 1.0F;
    constexpr int kFpsPosX = 10;
    constexpr int kFpsPosY = 10;
    constexpr int kTextPosX = 10;
    constexpr int kTextPosY = 40;
    constexpr int kTextFontSize = 20;
    constexpr int kTextLineSpacing = 30;
    constexpr int kKeySwitchScene = KEY_TAB;

    Camera3D createCamera() {
        Camera3D camera{};
        camera.position = Vector3{ kCameraPositionX, kCameraPositionY, kCameraPositionZ };
        camera.target = Vector3{ 0.0F, 0.0F, 0.0F };
        camera.up = Vector3{ 0.0F, 1.0F, 0.0F };
        camera.fovy = kCameraFovy;
        camera.projection = CAMERA_PERSPECTIVE;
        return camera;
    }

    void runGameLoop(Camera3D& camera, project::SceneManager& sceneManager) {
        SetTargetFPS(kTargetFPS);
        
        while (!WindowShouldClose()) {
            // Handle keyboard input for scene switching
            if (IsKeyPressed(kKeySwitchScene)) {
                sceneManager.switchToNextScene();
            }
            
            // Update camera
            UpdateCamera(&camera, CAMERA_ORBITAL);
            
            // Update current scene
            sceneManager.update();
            
            // Draw
            BeginDrawing();
            ClearBackground(SKYBLUE);
            
            BeginMode3D(camera);
            
            // Draw the current scene
            sceneManager.draw();
            
            // Draw a grid for reference
            DrawGrid(kGridSlices, kGridSpacing);
            
            // Draw origin axes
            DrawLine3D(Vector3{ 0.0F, 0.0F, 0.0F }, Vector3{ 1.0F, 0.0F, 0.0F }, RED);
            DrawLine3D(Vector3{ 0.0F, 0.0F, 0.0F }, Vector3{ 0.0F, 1.0F, 0.0F }, GREEN);
            DrawLine3D(Vector3{ 0.0F, 0.0F, 0.0F }, Vector3{ 0.0F, 0.0F, 1.0F }, BLUE);
            
            EndMode3D();
            
            // Draw UI
            DrawFPS(kFpsPosX, kFpsPosY);
            
            const auto* currentScene = sceneManager.getCurrentScene();
            const std::string sceneName = (currentScene != nullptr) ? currentScene->getName() : "No Scene";
            const size_t currentIndex = sceneManager.getCurrentSceneIndex();
            const size_t sceneCount = sceneManager.getSceneCount();
            const std::string sceneInfo = "Scene: " + sceneName + " (" + 
                                         std::to_string(currentIndex + 1) + 
                                         "/" + std::to_string(sceneCount) + ")";
            
            DrawText("3D Scene Example - Use mouse to orbit camera", kTextPosX, kTextPosY, kTextFontSize, DARKGRAY);
            DrawText(sceneInfo.c_str(), kTextPosX, kTextPosY + kTextLineSpacing, kTextFontSize, DARKGRAY);
            DrawText(("Press TAB to switch scenes"), kTextPosX, kTextPosY + (kTextLineSpacing * 2), kTextFontSize, DARKGRAY);
            
            EndDrawing();
        }
    }
}

int main() {
    InitWindow(kWindowWidth, kWindowHeight, kWindowTitle);
    
    if (!IsWindowReady()) {
        std::cerr << "Failed to initialize window\n";
        return 1;
    }
    
    // Set up 3D camera
    Camera3D camera = createCamera();
    
    // Create scene manager to handle multiple scenes (Strategy pattern)
    project::SceneManager sceneManager;
    
    // Register the tree scene (original scene)
    if (FileExists(kModelPath)) {
        auto treeScene = std::make_unique<project::TreeScene>(kModelPath);
        sceneManager.registerScene(std::move(treeScene));
        std::cout << "Registered Tree Scene\n";
    } else {
        std::cerr << "Warning: Model file not found: " << kModelPath << '\n';
        std::cerr << "Tree Scene will not be available\n";
    }
    
    // Register the geometric scene (new scene)
    auto geometricScene = std::make_unique<project::GeometricScene>();
    sceneManager.registerScene(std::move(geometricScene));
    std::cout << "Registered Geometric Scene\n";
    
    // Register the Bullet Physics scene
    auto bulletPhysicsScene = std::make_unique<project::BulletPhysicsScene>();
    sceneManager.registerScene(std::move(bulletPhysicsScene));
    std::cout << "Registered Bullet Physics Scene\n";
    
    std::cout << "Total scenes registered: " << sceneManager.getSceneCount() << '\n';
    std::cout << "Press TAB to switch between scenes\n";
    
    runGameLoop(camera, sceneManager);
    
    // Cleanup
    CloseWindow();
    return 0;
}

