#pragma once

#include <project/scene_strategy.hpp>
#include <project/ecs_components.hpp>
#include <project/ecs_systems.hpp>
#include <entt/entt.hpp>
#include <raylib.h>

// Forward declarations for Bullet Physics
class btDiscreteDynamicsWorld;
class btCollisionDispatcher;
class btDbvtBroadphase;
class btSequentialImpulseConstraintSolver;
class btDefaultCollisionConfiguration;

namespace project {

/// Scene demonstrating Bullet Physics integration with raylib using ECS
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
    [[nodiscard]] const char* getName() const override { return "Bullet Physics Scene (ECS)"; }
    void initialize() override;
    void cleanup() override;

private:
    // ECS registry
    entt::registry registry;
    
    // Bullet Physics world components
    btDiscreteDynamicsWorld* dynamicsWorld{nullptr};
    btCollisionDispatcher* dispatcher{nullptr};
    btDbvtBroadphase* overlappingPairCache{nullptr};
    btSequentialImpulseConstraintSolver* constraintSolver{nullptr};
    btDefaultCollisionConfiguration* collisionConfiguration{nullptr};
    
    bool isInitialized{false};
    
    // Helper functions
    void setupPhysicsWorld();
    void createGroundPlane();
    void createFallingBoxes();
    void cleanupPhysicsWorld();
    
    /// Helper to create a physics entity with all necessary components
    /// @param position Initial position
    /// @param collisionShape Bullet collision shape (ownership transferred)
    /// @param mass Mass of the body (0 for static)
    /// @param hasModel Whether a model is provided
    /// @param model Raylib model (only used if hasModel is true)
    /// @param color Color for rendering
    /// @param isGround Whether this is a ground entity
    /// @return The created entity
    entt::entity createPhysicsEntity(
        const Vector3& position,
        btCollisionShape* collisionShape,
        float mass,
        bool hasModel,
        const Model& model,
        const Color& color,
        bool isGround
    );
};

} // namespace project

