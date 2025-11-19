#pragma once

#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <string>

namespace project {

/// Represents a single object in a 3D scene with position, rotation, and scale
struct SceneObject {
    Model model;
    Vector3 position{0.0F, 0.0F, 0.0F};
    Vector3 rotation{0.0F, 0.0F, 0.0F};  // Rotation in degrees (pitch, yaw, roll)
    float scale{1.0F};
    Color tint{WHITE};
    std::string name;  // Optional identifier
    
    /// Draw this object in 3D space
    void draw() const;
    
    /// Get the transformation matrix for this object
    [[nodiscard]] Matrix getTransformMatrix() const;
};

/// Manages a collection of 3D objects and provides scene-level operations
class Scene {
public:
    Scene() = default;
    ~Scene();
    
    // Rule of Five: disable copy, allow move
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;
    Scene(Scene&&) noexcept = default;
    Scene& operator=(Scene&&) noexcept = default;
    
    /// Add an object to the scene (takes ownership of the model)
    /// Returns the index of the added object
    [[nodiscard]] size_t addObject(Model model, Vector3 position = Vector3Zero(), 
                                   float scale = 1.0F, const std::string& name = "");
    
    /// Add an object with full transform parameters
    [[nodiscard]] size_t addObject(Model model, Vector3 position, Vector3 rotation, 
                                   float scale, const std::string& name = "");
    
    /// Get a reference to an object by index
    [[nodiscard]] SceneObject& getObject(size_t index);
    [[nodiscard]] const SceneObject& getObject(size_t index) const;
    
    /// Get object by name (returns nullptr if not found)
    [[nodiscard]] SceneObject* findObjectByName(const std::string& name);
    [[nodiscard]] const SceneObject* findObjectByName(const std::string& name) const;
    
    /// Remove an object from the scene by index
    void removeObject(size_t index);
    
    /// Remove an object from the scene by name
    bool removeObjectByName(const std::string& name);
    
    /// Get the number of objects in the scene
    [[nodiscard]] size_t getObjectCount() const noexcept { return objects.size(); }
    
    /// Check if the scene is empty
    [[nodiscard]] bool isEmpty() const noexcept { return objects.empty(); }
    
    /// Draw all objects in the scene
    void draw() const;
    
    /// Clear all objects from the scene
    void clear();
    
private:
    std::vector<SceneObject> objects;
};

} // namespace project

