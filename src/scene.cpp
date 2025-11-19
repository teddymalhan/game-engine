#include <project/scene.hpp>
#include <algorithm>

namespace project {

void SceneObject::draw() const {
    // Convert Euler angles to rotation axis and angle for DrawModelEx
    float rotationAngle = 0.0F;
    Vector3 rotationAxis = Vector3Zero();
    
    if (rotation.x != 0.0F || rotation.z != 0.0F) {
        // Full Euler rotation: use matrix approach
        const auto rotX = MatrixRotateX(DEG2RAD * rotation.x);
        const auto rotY = MatrixRotateY(DEG2RAD * rotation.y);
        const auto rotZ = MatrixRotateZ(DEG2RAD * rotation.z);
        const auto combinedRot = MatrixMultiply(MatrixMultiply(rotZ, rotY), rotX);
        
        // Extract axis-angle from rotation matrix
        constexpr float kQuaternionToAngleMultiplier = 2.0F;
        const Quaternion quat = QuaternionFromMatrix(combinedRot);
        const Vector3 quatVec = Vector3{quat.x, quat.y, quat.z};
        rotationAxis = Vector3Normalize(quatVec);
        rotationAngle = kQuaternionToAngleMultiplier * acosf(quat.w);
    } else {
        // Simple Y-axis rotation (yaw only)
        rotationAngle = DEG2RAD * rotation.y;
        rotationAxis.x = 0.0F;
        rotationAxis.y = 1.0F;
        rotationAxis.z = 0.0F;
    }
    
    const auto scaleVec = Vector3{scale, scale, scale};
    DrawModelEx(model, position, rotationAxis, rotationAngle, scaleVec, tint);
}

Matrix SceneObject::getTransformMatrix() const {
    Matrix rotationMatrix = MatrixRotateXYZ(Vector3{
        DEG2RAD * rotation.x,
        DEG2RAD * rotation.y,
        DEG2RAD * rotation.z
    });
    
    Matrix scaleMatrix = MatrixScale(scale, scale, scale);
    Matrix translationMatrix = MatrixTranslate(position.x, position.y, position.z);
    
    return MatrixMultiply(MatrixMultiply(scaleMatrix, rotationMatrix), translationMatrix);
}

Scene::~Scene() {
    clear();
}

size_t Scene::addObject(Model model, Vector3 position, float scale, const std::string& name) {
    SceneObject obj;
    obj.model = model;
    obj.position = position;
    obj.scale = scale;
    obj.name = name;
    objects.push_back(obj);
    return objects.size() - 1;
}

size_t Scene::addObject(Model model, Vector3 position, Vector3 rotation, 
                       float scale, const std::string& name) {
    SceneObject obj;
    obj.model = model;
    obj.position = position;
    obj.rotation = rotation;
    obj.scale = scale;
    obj.name = name;
    objects.push_back(obj);
    return objects.size() - 1;
}

SceneObject& Scene::getObject(size_t index) {
    return objects.at(index);
}

const SceneObject& Scene::getObject(size_t index) const {
    return objects.at(index);
}

SceneObject* Scene::findObjectByName(const std::string& name) {
    auto iterator = std::find_if(objects.begin(), objects.end(), 
        [&name](const SceneObject& obj) { return obj.name == name; });
    return (iterator != objects.end()) ? &(*iterator) : nullptr;
}

const SceneObject* Scene::findObjectByName(const std::string& name) const {
    auto iterator = std::find_if(objects.begin(), objects.end(), 
        [&name](const SceneObject& obj) { return obj.name == name; });
    return (iterator != objects.end()) ? &(*iterator) : nullptr;
}

void Scene::removeObject(size_t index) {
    if (index < objects.size()) {
        UnloadModel(objects[index].model);
        const auto offset = static_cast<std::ptrdiff_t>(index);
        objects.erase(objects.begin() + offset);
    }
}

bool Scene::removeObjectByName(const std::string& name) {
    auto iterator = std::find_if(objects.begin(), objects.end(), 
        [&name](const SceneObject& obj) { return obj.name == name; });
    
    if (iterator != objects.end()) {
        UnloadModel(iterator->model);
        objects.erase(iterator);
        return true;
    }
    return false;
}

void Scene::draw() const {
    for (const auto& obj : objects) {
        obj.draw();
    }
}

void Scene::clear() {
    for (auto& obj : objects) {
        UnloadModel(obj.model);
    }
    objects.clear();
}

} // namespace project

