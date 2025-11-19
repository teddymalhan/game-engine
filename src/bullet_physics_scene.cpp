#include <project/bullet_physics_scene.hpp>
#include <btBulletDynamicsCommon.h>
#include <raymath.h>
#include <algorithm>
#include <cmath>

namespace project {

BulletPhysicsScene::BulletPhysicsScene() = default;

BulletPhysicsScene::~BulletPhysicsScene() {
    cleanup();
}

void BulletPhysicsScene::initialize() {
    if (isInitialized) {
        return;
    }
    
    // Create a simple cube model for drawing
    constexpr float kCubeSize = 1.0F;
    Mesh cubeMesh = GenMeshCube(kCubeSize, kCubeSize, kCubeSize);
    groundModel = LoadModelFromMesh(cubeMesh);
    hasGroundModel = IsModelValid(groundModel);
    
    setupPhysicsWorld();
    createGroundPlane();
    createFallingBoxes();
    
    isInitialized = true;
}

void BulletPhysicsScene::cleanup() {
    if (!isInitialized) {
        return;
    }
    
    // Mark as uninitialized first to prevent re-entry
    isInitialized = false;
    
    // Unload models (but don't clear physicsObjects yet - cleanupPhysicsWorld needs them)
    for (auto& obj : physicsObjects) {
        if (obj.hasModel) {
            UnloadModel(obj.model);
            obj.hasModel = false;
        }
    }
    
    if (hasGroundModel) {
        UnloadModel(groundModel);
        hasGroundModel = false;
    }
    
    // Cleanup physics world (this will remove rigid bodies from world and delete everything)
    cleanupPhysicsWorld();
}

void BulletPhysicsScene::setupPhysicsWorld() {
    // Create collision configuration
    collisionConfiguration = new btDefaultCollisionConfiguration();
    
    // Create dispatcher
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
    
    // Create broadphase
    overlappingPairCache = new btDbvtBroadphase();
    
    // Create constraint solver
    constraintSolver = new btSequentialImpulseConstraintSolver();
    
    // Create dynamics world
    dynamicsWorld = new btDiscreteDynamicsWorld(
        dispatcher,
        overlappingPairCache,
        constraintSolver,
        collisionConfiguration
    );
    
    // Set gravity (Y-axis down in Bullet, but we'll use Y-axis up like raylib)
    // Raylib uses Y-up, Bullet uses Y-up by default, so we set gravity to -9.8 in Y
    dynamicsWorld->setGravity(btVector3(0.0, -9.8, 0.0));
}

void BulletPhysicsScene::createGroundPlane() {
    // Create a large ground plane
    constexpr float kGroundHalfExtentsX = 20.0F;
    constexpr float kGroundHalfExtentsY = 0.5F;
    constexpr float kGroundHalfExtentsZ = 20.0F;
    constexpr float kGroundY = -0.5F;
    
    btCollisionShape* groundShape = new btBoxShape(
        btVector3(kGroundHalfExtentsX, kGroundHalfExtentsY, kGroundHalfExtentsZ)
    );
    
    btTransform groundTransform;
    groundTransform.setIdentity();
    groundTransform.setOrigin(btVector3(0.0, kGroundY, 0.0));
    
    btScalar mass = 0.0; // Static object (mass = 0)
    btVector3 localInertia(0.0, 0.0, 0.0);
    
    btDefaultMotionState* groundMotionState = new btDefaultMotionState(groundTransform);
    btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(
        mass,
        groundMotionState,
        groundShape,
        localInertia
    );
    
    btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
    dynamicsWorld->addRigidBody(groundRigidBody);
    
    // Store ground in physics objects (for cleanup)
    PhysicsObject groundObj;
    groundObj.rigidBody = groundRigidBody;
    groundObj.collisionShape = groundShape;
    groundObj.motionState = groundMotionState;
    groundObj.hasModel = hasGroundModel;
    if (hasGroundModel) {
        groundObj.model = groundModel;
    }
    groundObj.color = DARKGREEN;
    physicsObjects.push_back(groundObj);
}

void BulletPhysicsScene::createFallingBoxes() {
    constexpr int kBoxCount = 10;
    constexpr float kBoxSize = 0.5F;
    constexpr float kBoxMass = 1.0F;
    constexpr float kStartHeight = 5.0F;
    constexpr float kSpacing = 2.0F;
    
    // Create a cube mesh for the boxes
    Mesh cubeMesh = GenMeshCube(kBoxSize * 2.0F, kBoxSize * 2.0F, kBoxSize * 2.0F);
    
    // Create boxes in a grid pattern
    const int kGridSize = static_cast<int>(std::sqrt(static_cast<float>(kBoxCount)));
    int boxIndex = 0;
    
    for (int i = 0; i < kGridSize && boxIndex < kBoxCount; ++i) {
        for (int j = 0; j < kGridSize && boxIndex < kBoxCount; ++j) {
            // Create collision shape
            btCollisionShape* boxShape = new btBoxShape(
                btVector3(kBoxSize, kBoxSize, kBoxSize)
            );
            
            // Calculate position
            const float posX = (static_cast<float>(i) - static_cast<float>(kGridSize) / 2.0F) * kSpacing;
            const float posZ = (static_cast<float>(j) - static_cast<float>(kGridSize) / 2.0F) * kSpacing;
            
            btTransform boxTransform;
            boxTransform.setIdentity();
            boxTransform.setOrigin(btVector3(posX, kStartHeight, posZ));
            
            // Calculate inertia
            btScalar mass = kBoxMass;
            btVector3 localInertia(0.0, 0.0, 0.0);
            boxShape->calculateLocalInertia(mass, localInertia);
            
            // Create motion state
            btDefaultMotionState* boxMotionState = new btDefaultMotionState(boxTransform);
            
            // Create rigid body
            btRigidBody::btRigidBodyConstructionInfo boxRigidBodyCI(
                mass,
                boxMotionState,
                boxShape,
                localInertia
            );
            
            btRigidBody* boxRigidBody = new btRigidBody(boxRigidBodyCI);
            dynamicsWorld->addRigidBody(boxRigidBody);
            
            // Create model
            Model boxModel = LoadModelFromMesh(cubeMesh);
            const bool hasBoxModel = IsModelValid(boxModel);
            
            // Vary colors
            const float hue = static_cast<float>(boxIndex) / static_cast<float>(kBoxCount);
            const Color boxColor = ColorFromHSV(hue * 360.0F, 0.8F, 0.9F);
            
            // Store physics object
            PhysicsObject boxObj;
            boxObj.rigidBody = boxRigidBody;
            boxObj.collisionShape = boxShape;
            boxObj.motionState = boxMotionState;
            boxObj.model = boxModel;
            boxObj.hasModel = hasBoxModel;
            boxObj.color = boxColor;
            physicsObjects.push_back(boxObj);
            
            ++boxIndex;
        }
    }
}

void BulletPhysicsScene::update() {
    if (!isInitialized || dynamicsWorld == nullptr) {
        return;
    }
    
    // Step the physics simulation
    // Use fixed timestep for consistent physics
    constexpr float kFixedTimeStep = 1.0F / 60.0F;
    constexpr int kMaxSubSteps = 10;
    const float deltaTime = GetFrameTime();
    
    dynamicsWorld->stepSimulation(deltaTime, kMaxSubSteps);
}

void BulletPhysicsScene::draw() const {
    if (!isInitialized) {
        return;
    }
    
    // Draw ground plane
    if (hasGroundModel) {
        constexpr Vector3 kGroundPosition = {0.0F, -0.5F, 0.0F};
        constexpr Vector3 kGroundScale = {40.0F, 1.0F, 40.0F};
        DrawModelEx(groundModel, kGroundPosition, Vector3{0.0F, 1.0F, 0.0F}, 0.0F, kGroundScale, DARKGREEN);
    }
    
    // Draw physics objects
    for (const auto& obj : physicsObjects) {
        if (obj.rigidBody == nullptr || !obj.hasModel) {
            continue;
        }
        
        // Get transform from Bullet
        btTransform transform;
        if (obj.rigidBody->getMotionState()) {
            obj.rigidBody->getMotionState()->getWorldTransform(transform);
        } else {
            transform = obj.rigidBody->getWorldTransform();
        }
        
        // Convert Bullet transform to raylib
        const btVector3& origin = transform.getOrigin();
        const Vector3 position = Vector3{
            static_cast<float>(origin.x()),
            static_cast<float>(origin.y()),
            static_cast<float>(origin.z())
        };
        
        // Get rotation from Bullet quaternion
        const btQuaternion& rotation = transform.getRotation();
        const Quaternion quat = Quaternion{
            static_cast<float>(rotation.x()),
            static_cast<float>(rotation.y()),
            static_cast<float>(rotation.z()),
            static_cast<float>(rotation.w())
        };
        
        // Convert quaternion to axis-angle for DrawModelEx
        const Vector3 rotationAxis = Vector3Normalize(Vector3{quat.x, quat.y, quat.z});
        const float rotationAngle = 2.0F * acosf(quat.w);
        
        // Draw the model
        constexpr float kBoxScale = 1.0F;
        DrawModelEx(obj.model, position, rotationAxis, rotationAngle, Vector3{kBoxScale, kBoxScale, kBoxScale}, obj.color);
        
        // Draw wireframe
        constexpr float kBoxSize = 0.5F;
        DrawCubeWiresV(position, Vector3{kBoxSize * 2.0F, kBoxSize * 2.0F, kBoxSize * 2.0F}, DARKGRAY);
    }
}

void BulletPhysicsScene::cleanupPhysicsWorld() {
    // Remove and delete all rigid bodies
    for (auto& obj : physicsObjects) {
        if (obj.rigidBody != nullptr && dynamicsWorld != nullptr) {
            dynamicsWorld->removeRigidBody(obj.rigidBody);
        }
        
        if (obj.rigidBody != nullptr) {
            delete obj.rigidBody;
            obj.rigidBody = nullptr;
        }
        
        if (obj.motionState != nullptr) {
            delete obj.motionState;
            obj.motionState = nullptr;
        }
        
        if (obj.collisionShape != nullptr) {
            delete obj.collisionShape;
            obj.collisionShape = nullptr;
        }
    }
    
    physicsObjects.clear();
    
    // Delete dynamics world components
    if (dynamicsWorld != nullptr) {
        delete dynamicsWorld;
        dynamicsWorld = nullptr;
    }
    
    if (constraintSolver != nullptr) {
        delete constraintSolver;
        constraintSolver = nullptr;
    }
    
    if (overlappingPairCache != nullptr) {
        delete overlappingPairCache;
        overlappingPairCache = nullptr;
    }
    
    if (dispatcher != nullptr) {
        delete dispatcher;
        dispatcher = nullptr;
    }
    
    if (collisionConfiguration != nullptr) {
        delete collisionConfiguration;
        collisionConfiguration = nullptr;
    }
}

} // namespace project

