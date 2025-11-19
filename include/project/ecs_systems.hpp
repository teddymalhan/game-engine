#pragma once

#include <project/ecs_components.hpp>
#include <entt/entt.hpp>
#include <btBulletDynamicsCommon.h>
#include <raymath.h>
#include <cmath>

namespace project {

/// Physics system: synchronizes Bullet Physics transforms to Transform components
class PhysicsSystem {
public:
    /// Update physics simulation and sync transforms
    /// @param registry The ECS registry
    /// @param dynamicsWorld The Bullet Physics dynamics world
    /// @param deltaTime Time since last frame
    static void update(entt::registry& registry, btDiscreteDynamicsWorld* dynamicsWorld, float deltaTime) {
        if (dynamicsWorld == nullptr) {
            return;
        }
        
        // Step the physics simulation
        constexpr float kFixedTimeStep = 1.0F / 60.0F;
        constexpr int kMaxSubSteps = 10;
        dynamicsWorld->stepSimulation(deltaTime, kMaxSubSteps);
        
        // Sync Bullet Physics transforms to Transform components
        auto view = registry.view<Transform, PhysicsBody>();
        for (auto entity : view) {
            auto& transform = view.get<Transform>(entity);
            const auto& physicsBody = view.get<PhysicsBody>(entity);
            
            if (physicsBody.rigidBody == nullptr) {
                continue;
            }
            
            // Get transform from Bullet
            btTransform bulletTransform;
            if (physicsBody.rigidBody->getMotionState() != nullptr) {
                physicsBody.rigidBody->getMotionState()->getWorldTransform(bulletTransform);
            } else {
                bulletTransform = physicsBody.rigidBody->getWorldTransform();
            }
            
            // Convert Bullet position to raylib
            const btVector3& origin = bulletTransform.getOrigin();
            transform.position = Vector3{
                static_cast<float>(origin.x()),
                static_cast<float>(origin.y()),
                static_cast<float>(origin.z())
            };
            
            // Convert Bullet quaternion to raylib (Bullet: w, x, y, z; raylib: x, y, z, w)
            const btQuaternion& bulletQuat = bulletTransform.getRotation();
            transform.rotation = Quaternion{
                static_cast<float>(bulletQuat.x()),
                static_cast<float>(bulletQuat.y()),
                static_cast<float>(bulletQuat.z()),
                static_cast<float>(bulletQuat.w())
            };
        }
    }
};

/// Render system: draws entities with Renderable components
class RenderSystem {
public:
    /// Draw all renderable entities
    /// @param registry The ECS registry
    static void draw(const entt::registry& registry) {
        auto view = registry.view<const Transform, const Renderable>();
        
        for (auto entity : view) {
            const auto& transform = view.get<Transform>(entity);
            const auto& renderable = view.get<Renderable>(entity);
            
            if (!renderable.hasModel) {
                continue;
            }
            
            // Convert quaternion to axis-angle for DrawModelEx
            const Vector3 rotationAxis = Vector3Normalize(Vector3{
                transform.rotation.x,
                transform.rotation.y,
                transform.rotation.z
            });
            const float rotationAngle = 2.0F * acosf(transform.rotation.w);
            
            // Draw the model
            DrawModelEx(
                renderable.model,
                transform.position,
                rotationAxis,
                rotationAngle,
                transform.scale,
                renderable.color
            );
            
            // Draw wireframe for physics objects (optional visual aid)
            // You can add a component to control this if needed
        }
    }
    
    /// Draw ground entities with special handling
    /// @param registry The ECS registry
    static void drawGround(const entt::registry& registry) {
        auto view = registry.view<const Transform, const Renderable, const Ground>();
        
        for (auto entity : view) {
            const auto& transform = view.get<Transform>(entity);
            const auto& renderable = view.get<Renderable>(entity);
            
            if (!renderable.hasModel) {
                continue;
            }
            
            // Ground is typically drawn with a specific scale
            DrawModelEx(
                renderable.model,
                transform.position,
                Vector3{0.0F, 1.0F, 0.0F},  // Up axis
                0.0F,  // No rotation
                transform.scale,
                renderable.color
            );
        }
    }
};

} // namespace project

