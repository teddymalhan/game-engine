#include <raylib.h>
#include <raymath.h>
#include <project/scene.hpp>
#include <iostream>

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
    constexpr float kModelScale = 2.0F;
    constexpr int kGridSlices = 10;
    constexpr float kGridSpacing = 1.0F;
    constexpr float kTestCubePosX = 2.0F;
    constexpr float kTestCubeSize = 0.5F;
    constexpr int kFpsPosX = 10;
    constexpr int kFpsPosY = 10;
    constexpr int kTextPosX = 10;
    constexpr int kTextPosY = 40;
    constexpr int kTextFontSize = 20;
    constexpr int kTextLineSpacing = 30;

    Camera3D createCamera() {
        Camera3D camera{};
        camera.position = Vector3{ kCameraPositionX, kCameraPositionY, kCameraPositionZ };
        camera.target = Vector3{ 0.0F, 0.0F, 0.0F };
        camera.up = Vector3{ 0.0F, 1.0F, 0.0F };
        camera.fovy = kCameraFovy;
        camera.projection = CAMERA_PERSPECTIVE;
        return camera;
    }

    void runGameLoop(Camera3D& camera, project::Scene& scene) {
        SetTargetFPS(kTargetFPS);
        
        while (!WindowShouldClose()) {
            // Update camera
            UpdateCamera(&camera, CAMERA_ORBITAL);
            
            // Draw
            BeginDrawing();
            ClearBackground(SKYBLUE);
            
            BeginMode3D(camera);
            
            // Draw a test cube to verify 3D rendering works (offset to the side)
            DrawCube(Vector3{ kTestCubePosX, 0.0F, 0.0F }, kTestCubeSize, kTestCubeSize, kTestCubeSize, RED);
            DrawCubeWires(Vector3{ kTestCubePosX, 0.0F, 0.0F }, kTestCubeSize, kTestCubeSize, kTestCubeSize, MAROON);
            
            // Draw the entire scene (all objects)
            scene.draw();
            
            // Draw a grid for reference
            DrawGrid(kGridSlices, kGridSpacing);
            
            // Draw origin axes
            DrawLine3D(Vector3{ 0.0F, 0.0F, 0.0F }, Vector3{ 1.0F, 0.0F, 0.0F }, RED);
            DrawLine3D(Vector3{ 0.0F, 0.0F, 0.0F }, Vector3{ 0.0F, 1.0F, 0.0F }, GREEN);
            DrawLine3D(Vector3{ 0.0F, 0.0F, 0.0F }, Vector3{ 0.0F, 0.0F, 1.0F }, BLUE);
            
            EndMode3D();
            
            // Draw UI
            DrawFPS(kFpsPosX, kFpsPosY);
            DrawText("3D Scene Example - Use mouse to orbit camera", kTextPosX, kTextPosY, kTextFontSize, DARKGRAY);
            DrawText(("Objects in scene: " + std::to_string(scene.getObjectCount())).c_str(), 
                     kTextPosX, kTextPosY + kTextLineSpacing, kTextFontSize, DARKGRAY);
            
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
    
    // Check if model file exists
    if (!FileExists(kModelPath)) {
        std::cerr << "Model file not found: " << kModelPath << '\n';
        CloseWindow();
        return 1;
    }
    
    // Set up 3D camera
    Camera3D camera = createCamera();
    
    // Create a scene to manage multiple objects
    project::Scene scene;
    
    // Load the 3D model
    std::cout << "Loading model: " << kModelPath << '\n';
    Model model = LoadModel(kModelPath);
    
    if (!IsModelValid(model)) {
        std::cerr << "Failed to load model: " << kModelPath << '\n';
        std::cerr << "Model mesh count: " << model.meshCount << '\n';
        CloseWindow();
        return 1;
    }
    
    std::cout << "Model loaded successfully!\n";
    std::cout << "Mesh count: " << model.meshCount << '\n';
    std::cout << "Material count: " << model.materialCount << '\n';
    
    // Get model bounding box to understand its size
    BoundingBox bounds = GetModelBoundingBox(model);
    std::cout << "Model bounds - Min: (" << bounds.min.x << ", " << bounds.min.y << ", " << bounds.min.z << ")\n";
    std::cout << "Model bounds - Max: (" << bounds.max.x << ", " << bounds.max.y << ", " << bounds.max.z << ")\n";
    
    // Add the model to the scene at the origin
    const size_t objectIndex = scene.addObject(model, Vector3{0.0F, 0.0F, 0.0F}, kModelScale, "tree-main");
    (void)objectIndex;  // Suppress unused variable warning
    
    // Example: Add multiple instances of the same model at different positions
    // (Note: In a real scenario, you'd want to load the model multiple times or use instancing)
    // For now, we'll just add one object. You can add more like this:
    // scene.addObject(anotherModel, Vector3{2.0F, 0.0F, 0.0F}, 1.0F, "tree-2");
    
    std::cout << "Scene created with " << scene.getObjectCount() << " object(s)\n";
    
    runGameLoop(camera, scene);
    
    // Cleanup - Scene destructor will handle unloading models
    CloseWindow();
    return 0;
}

