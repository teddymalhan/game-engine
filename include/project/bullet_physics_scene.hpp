#pragma once

#include <project/scene_strategy.hpp>
#include <raylib.h>
#include <vector>

// Forward declarations for Bullet Physics
class btDiscreteDynamicsWorld;
class btCollisionDispatcher;
class btDbvtBroadphase;
class btSequentialImpulseConstraintSolver;
class btDefaultCollisionConfiguration;
class btRigidBody;
class btCollisionShape;
class btDefaultMotionState;

namespace project {

/// Scene demonstrating Bullet Physics integration with raylib
/// Features falling boxes, a ground plane, and real-time physics simulation
class BulletPhysicsScene : public SceneStrategy {
public:
    BulletPhysicsScene();
    ~BulletPhysicsScene() override;
    
    // Rule of Five: disable copy, allow move
    BulletPhysicsScene(const BulletPhysicsScene&) = delete;
    BulletPhysicsScene& operator=(const BulletPhysicsScene&) = delete;
    BulletPhysicsScene(BulletPhysicsScene&&) noexcept = default;
    BulletPhysicsScene& operator=(BulletPhysicsScene&&) noexcept = default;
    
    void update() override;
    void draw() const override;
    [[nodiscard]] const char* getName() const override { return "Bullet Physics Scene"; }
    void initialize() override;
    void cleanup() override;

private:
    // Bullet Physics world components
    btDiscreteDynamicsWorld* dynamicsWorld{nullptr};
    btCollisionDispatcher* dispatcher{nullptr};
    btDbvtBroadphase* overlappingPairCache{nullptr};
    btSequentialImpulseConstraintSolver* constraintSolver{nullptr};
    btDefaultCollisionConfiguration* collisionConfiguration{nullptr};
    
    // Physics objects
    struct PhysicsObject {
        btRigidBody* rigidBody{nullptr};
        btCollisionShape* collisionShape{nullptr};
        btDefaultMotionState* motionState{nullptr};
        Model model{};
        bool hasModel{false};
        Color color{WHITE};
    };
    
    std::vector<PhysicsObject> physicsObjects;
    Model groundModel{};
    bool hasGroundModel{false};
    bool isInitialized{false};
    
    // Helper functions
    void setupPhysicsWorld();
    void createGroundPlane();
    void createFallingBoxes();
    void cleanupPhysicsWorld();
};

} // namespace project

