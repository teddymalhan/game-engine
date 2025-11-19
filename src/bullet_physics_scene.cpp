#include <project/bullet_physics_scene.hpp>
#include <btBulletDynamicsCommon.h>
#include <raymath.h>
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
    
    setupPhysicsWorld();
    createGroundPlane();
    createCharacter();
    
    isInitialized = true;
}

void BulletPhysicsScene::cleanup() {
    if (!isInitialized) {
        return;
    }
    
    // Mark as uninitialized first to prevent re-entry
    isInitialized = false;
    
    // Cleanup all entities - components will handle their own cleanup
    // First remove all rigid bodies from the physics world
    auto physicsView = registry.view<PhysicsBody>();
    for (auto entity : physicsView) {
        const auto& physicsBody = physicsView.get<PhysicsBody>(entity);
        if (physicsBody.rigidBody != nullptr && dynamicsWorld != nullptr) {
            dynamicsWorld->removeRigidBody(physicsBody.rigidBody);
        }
    }
    
    // Clear the registry (this will destroy all entities and components)
    registry.clear();
    
    // Cleanup physics world
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
    
    // Set gravity (Y-axis up in raylib, Y-axis up in Bullet by default)
    constexpr float kGravity = -9.8F;
    dynamicsWorld->setGravity(btVector3(0.0F, kGravity, 0.0F));
}

void BulletPhysicsScene::createGroundPlane() {
    // Create a large ground plane
    constexpr float kGroundHalfExtentsX = 20.0F;
    constexpr float kGroundHalfExtentsY = 0.5F;
    constexpr float kGroundHalfExtentsZ = 20.0F;
    constexpr float kGroundY = -0.5F;
    
    // Create collision shape
    btCollisionShape* groundShape = new btBoxShape(
        btVector3(kGroundHalfExtentsX, kGroundHalfExtentsY, kGroundHalfExtentsZ)
    );
    
    // Create ground model
    constexpr float kCubeSize = 1.0F;
    Mesh cubeMesh = GenMeshCube(kCubeSize, kCubeSize, kCubeSize);
    Model groundModel = LoadModelFromMesh(cubeMesh);
    const bool hasGroundModel = IsModelValid(groundModel);
    
    // Create ground entity
    const Vector3 groundPosition{0.0F, kGroundY, 0.0F};
    const Vector3 groundScale{40.0F, 1.0F, 40.0F};
    
    auto groundEntity = createPhysicsEntity(
        groundPosition,
        groundShape,
        0.0F,  // Static (mass = 0)
        hasGroundModel,
        groundModel,
        DARKGREEN,
        true  // isGround
    );
    
    // Set ground scale
    if (registry.all_of<Transform>(groundEntity)) {
        auto& transform = registry.get<Transform>(groundEntity);
        transform.scale = groundScale;
    }
}

void BulletPhysicsScene::createFallingBoxes() {
    constexpr int kBoxCount = 10;
    constexpr float kBoxSize = 0.5F;
    constexpr float kBoxMass = 1.0F;
    constexpr float kStartHeight = 5.0F;
    constexpr float kSpacing = 2.0F;
    
    // Create a cube mesh for the boxes (shared mesh)
    Mesh cubeMesh = GenMeshCube(kBoxSize * 2.0F, kBoxSize * 2.0F, kBoxSize * 2.0F);
    
    // Create boxes in a grid pattern
    const int kGridSize = static_cast<int>(std::sqrt(static_cast<float>(kBoxCount)));
    int boxIndex = 0;
    
    for (int i = 0; i < kGridSize && boxIndex < kBoxCount; ++i) {
        for (int j = 0; j < kGridSize && boxIndex < kBoxCount; ++j) {
            // Calculate position
            const float posX = (static_cast<float>(i) - static_cast<float>(kGridSize) / 2.0F) * kSpacing;
            const float posZ = (static_cast<float>(j) - static_cast<float>(kGridSize) / 2.0F) * kSpacing;
            
            // Create collision shape
            btCollisionShape* boxShape = new btBoxShape(
                btVector3(kBoxSize, kBoxSize, kBoxSize)
            );
            
            // Create model (each box gets its own model instance)
            Model boxModel = LoadModelFromMesh(cubeMesh);
            const bool hasBoxModel = IsModelValid(boxModel);
            
            // Vary colors
            const float hue = static_cast<float>(boxIndex) / static_cast<float>(kBoxCount);
            const Color boxColor = ColorFromHSV(hue * 360.0F, 0.8F, 0.9F);
            
            // Create box entity
            const Vector3 boxPosition{posX, kStartHeight, posZ};
            createPhysicsEntity(
                boxPosition,
                boxShape,
                kBoxMass,
                hasBoxModel,
                boxModel,
                boxColor,
                false  // not ground
            );
            
            ++boxIndex;
        }
    }
}

void BulletPhysicsScene::createCharacter() {
    constexpr const char* kCharacterPath = "assets/characters/character-a.glb";
    constexpr float kCharacterMass = 0.0F;  // Static (doesn't fall)
    constexpr float kCharacterDistance = 5.0F;  // Distance from origin (camera looks at origin)
    constexpr float kGroundY = -0.0F;  // Ground plane Y position
    
    // Check if model file exists
    if (!FileExists(kCharacterPath)) {
        return;  // Silently skip if file doesn't exist
    }
    
    // Load the character model
    Model characterModel = LoadModel(kCharacterPath);
    const bool hasCharacterModel = IsModelValid(characterModel);
    
    if (!hasCharacterModel) {
        return;  // Silently skip if model failed to load
    }
    
    // Get bounding box to determine collision shape size
    const BoundingBox boundingBox = GetModelBoundingBox(characterModel);
    const Vector3 boundingSize = Vector3Subtract(boundingBox.max, boundingBox.min);
    
    // Calculate capsule dimensions from bounding box
    // Use the average of width and depth for radius, and height for capsule height
    const float capsuleRadius = std::max(boundingSize.x, boundingSize.z) * 0.5F;
    const float capsuleHeight = std::max(boundingSize.y - (capsuleRadius * 2.0F), 0.1F);
    
    // Create capsule collision shape (better for characters than boxes)
    // btCapsuleShape is Y-axis aligned by default (upright for characters)
    btCollisionShape* capsuleShape = new btCapsuleShape(
        static_cast<btScalar>(capsuleRadius),
        static_cast<btScalar>(capsuleHeight)
    );
    
    // Position character on the ground, away from camera
    // Camera is at (2, 1.5, 2) looking at (0, 0, 0)
    // Place character in front of camera view, on the ground
    // Ground is at Y = -0.5, so character center should be at ground level + half capsule height
    const float characterY = kGroundY + (capsuleHeight * 0.5F) + capsuleRadius;
    const Vector3 characterPosition{0.0F, characterY, -kCharacterDistance};
    
    const auto characterEntity = createPhysicsEntity(
        characterPosition,
        capsuleShape,
        kCharacterMass,
        hasCharacterModel,
        characterModel,
        WHITE,  // Use model's original colors
        false   // not ground
    );
    
    // Add Name component to identify the character
    registry.emplace<Name>(characterEntity, "character-a");
}

entt::entity BulletPhysicsScene::createPhysicsEntity(
    const Vector3& position,
    btCollisionShape* collisionShape,
    float mass,
    bool hasModel,
    const Model& model,
    const Color& color,
    bool isGround
) {
    // Create entity
    const auto entity = registry.create();
    
    // Add Transform component
    auto& transform = registry.emplace<Transform>(entity);
    transform.position = position;
    transform.rotation = Quaternion{0.0F, 0.0F, 0.0F, 1.0F};  // Identity quaternion (x, y, z, w)
    transform.scale = Vector3{1.0F, 1.0F, 1.0F};
    
    // Setup Bullet Physics transform
    btTransform bulletTransform;
    bulletTransform.setIdentity();
    bulletTransform.setOrigin(btVector3(position.x, position.y, position.z));
    
    // Calculate inertia
    btVector3 localInertia(0.0, 0.0, 0.0);
    const bool isStatic = (mass == 0.0F);
    if (!isStatic) {
        collisionShape->calculateLocalInertia(mass, localInertia);
    }
    
    // Create motion state
    btDefaultMotionState* motionState = new btDefaultMotionState(bulletTransform);
    
    // Create rigid body
    btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(
        mass,
        motionState,
        collisionShape,
        localInertia
    );
    
    btRigidBody* rigidBody = new btRigidBody(rigidBodyCI);
    
    // Add rigid body to physics world
    if (dynamicsWorld != nullptr) {
        dynamicsWorld->addRigidBody(rigidBody);
    }
    
    // Add PhysicsBody component
    auto& physicsBody = registry.emplace<PhysicsBody>(entity);
    physicsBody.rigidBody = rigidBody;
    physicsBody.collisionShape = collisionShape;
    physicsBody.motionState = motionState;
    physicsBody.mass = mass;
    physicsBody.isStatic = isStatic;
    
    // Add Renderable component if model is provided
    if (hasModel) {
        auto& renderable = registry.emplace<Renderable>(entity);
        renderable.model = model;
        renderable.color = color;
        renderable.hasModel = IsModelValid(model);
    }
    
    // Add Ground tag if this is ground
    if (isGround) {
        registry.emplace<Ground>(entity);
    }
    
    return entity;
}

void BulletPhysicsScene::update() {
    if (!isInitialized || dynamicsWorld == nullptr) {
        return;
    }
    
    // Update physics system (steps simulation and syncs transforms)
    const float deltaTime = GetFrameTime();
    PhysicsSystem::update(registry, dynamicsWorld, deltaTime);
}

void BulletPhysicsScene::draw() const {
    if (!isInitialized) {
        return;
    }
    
    // Draw ground entities first
    RenderSystem::drawGround(registry);
    
    // Draw all other renderable entities
    RenderSystem::draw(registry);
    
    // Draw wireframes for physics objects (optional visual aid)
    auto view = registry.view<const Transform, const PhysicsBody>(entt::exclude<Ground>);
    for (auto entity : view) {
        const auto& transform = view.get<Transform>(entity);
        constexpr float kBoxSize = 0.5F;
        DrawCubeWiresV(
            transform.position,
            Vector3{kBoxSize * 2.0F, kBoxSize * 2.0F, kBoxSize * 2.0F},
            DARKGRAY
        );
    }
}

void BulletPhysicsScene::cleanupPhysicsWorld() {
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
