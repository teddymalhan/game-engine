#pragma once

#include <entt/entt.hpp>
#include <raylib.h>
#include <raymath.h>
#include <btBulletDynamicsCommon.h>
#include <memory>
#include <string>

namespace project {

/// Transform component: position, rotation, and scale in 3D space
struct Transform {
    Vector3 position{0.0F, 0.0F, 0.0F};
    Quaternion rotation{0.0F, 0.0F, 0.0F, 1.0F};  // x, y, z, w (raylib format)
    Vector3 scale{1.0F, 1.0F, 1.0F};
    
    /// Get transformation matrix
    [[nodiscard]] Matrix getMatrix() const {
        return MatrixScale(scale.x, scale.y, scale.z) *
               QuaternionToMatrix(rotation) *
               MatrixTranslate(position.x, position.y, position.z);
    }
};

/// Physics body component: wraps Bullet Physics rigid body
/// Manages the lifetime of Bullet Physics objects
struct PhysicsBody {
    btRigidBody* rigidBody{nullptr};
    btCollisionShape* collisionShape{nullptr};
    btDefaultMotionState* motionState{nullptr};
    float mass{0.0F};
    bool isStatic{false};
    
    /// Cleanup Bullet Physics objects
    void cleanup() {
        if (rigidBody != nullptr) {
            delete rigidBody;
            rigidBody = nullptr;
        }
        if (motionState != nullptr) {
            delete motionState;
            motionState = nullptr;
        }
        if (collisionShape != nullptr) {
            delete collisionShape;
            collisionShape = nullptr;
        }
    }
    
    ~PhysicsBody() {
        cleanup();
    }
    
    // Rule of Five: disable copy, allow move
    PhysicsBody(const PhysicsBody&) = delete;
    PhysicsBody& operator=(const PhysicsBody&) = delete;
    PhysicsBody(PhysicsBody&& other) noexcept
        : rigidBody(other.rigidBody)
        , collisionShape(other.collisionShape)
        , motionState(other.motionState)
        , mass(other.mass)
        , isStatic(other.isStatic) {
        other.rigidBody = nullptr;
        other.collisionShape = nullptr;
        other.motionState = nullptr;
    }
    
    PhysicsBody& operator=(PhysicsBody&& other) noexcept {
        if (this != &other) {
            cleanup();
            rigidBody = other.rigidBody;
            collisionShape = other.collisionShape;
            motionState = other.motionState;
            mass = other.mass;
            isStatic = other.isStatic;
            other.rigidBody = nullptr;
            other.collisionShape = nullptr;
            other.motionState = nullptr;
        }
        return *this;
    }
    
    PhysicsBody() = default;
};

/// Renderable component: visual representation of an entity
struct Renderable {
    Model model{};
    Color color{WHITE};
    bool hasModel{false};
    
    /// Cleanup raylib model
    void cleanup() {
        if (hasModel) {
            UnloadModel(model);
            hasModel = false;
        }
    }
    
    ~Renderable() {
        cleanup();
    }
    
    // Rule of Five: disable copy, allow move
    Renderable(const Renderable&) = delete;
    Renderable& operator=(const Renderable&) = delete;
    Renderable(Renderable&& other) noexcept
        : model(other.model)
        , color(other.color)
        , hasModel(other.hasModel) {
        other.hasModel = false;
    }
    
    Renderable& operator=(Renderable&& other) noexcept {
        if (this != &other) {
            cleanup();
            model = other.model;
            color = other.color;
            hasModel = other.hasModel;
            other.hasModel = false;
        }
        return *this;
    }
    
    Renderable() = default;
};

/// Ground tag component: marks an entity as ground/static surface
struct Ground {};

/// Name component: optional identifier for entities
struct Name {
    std::string value;
    
    Name() = default;
    explicit Name(std::string name) : value(std::move(name)) {}
};

} // namespace project

