#include <project/scene_manager.hpp>
#include <stdexcept>
#include <iostream>

namespace project {

SceneManager::SceneManager() = default;

void SceneManager::registerScene(std::unique_ptr<SceneStrategy> scene) {
    if (!scene) {
        return;
    }
    
    scenes.push_back(std::move(scene));
    
    // If this is the first scene, activate it
    if (scenes.size() == 1) {
        activateScene(0);
    }
}

void SceneManager::switchToScene(size_t index) {
    if (index >= scenes.size()) {
        return;
    }
    
    activateScene(index);
}

void SceneManager::switchToNextScene() {
    if (scenes.empty()) {
        return;
    }
    
    // Ensure currentSceneIndex is valid
    if (currentSceneIndex >= scenes.size()) {
        std::cerr << "Error: currentSceneIndex (" << currentSceneIndex 
                  << ") is out of bounds. Resetting to 0.\n";
        currentSceneIndex = 0;
    }
    
    // Calculate next index with proper wrapping
    // When at the last scene (index scenes.size() - 1), this should wrap to 0
    const size_t nextIndex = (currentSceneIndex + 1) % scenes.size();
    
    // Double-check the calculation is correct (should never happen with modulo, but be safe)
    if (nextIndex >= scenes.size()) {
        std::cerr << "Error: Calculated nextIndex (" << nextIndex 
                  << ") is out of bounds. Forcing to 0.\n";
        activateScene(0);
        return;
    }
    
    activateScene(nextIndex);
}

void SceneManager::switchToPreviousScene() {
    if (scenes.empty()) {
        return;
    }
    
    const size_t prevIndex = (currentSceneIndex == 0) 
        ? scenes.size() - 1 
        : currentSceneIndex - 1;
    activateScene(prevIndex);
}

SceneStrategy* SceneManager::getCurrentScene() noexcept {
    if (currentSceneIndex >= scenes.size()) {
        return nullptr;
    }
    return scenes[currentSceneIndex].get();
}

const SceneStrategy* SceneManager::getCurrentScene() const noexcept {
    if (currentSceneIndex >= scenes.size()) {
        return nullptr;
    }
    return scenes[currentSceneIndex].get();
}

void SceneManager::update() {
    if (auto* scene = getCurrentScene()) {
        scene->update();
    }
}

void SceneManager::draw() const {
    if (const auto* scene = getCurrentScene()) {
        scene->draw();
    }
}

void SceneManager::activateScene(size_t index) {
    if (index >= scenes.size()) {
        std::cerr << "Error: Attempted to activate scene " << index 
                  << " but only " << scenes.size() << " scenes exist\n";
        return;
    }
    
    // If switching to the same scene, do nothing
    if (index == currentSceneIndex) {
        return;
    }
    
    // Store the old index before updating
    const size_t oldIndex = currentSceneIndex;
    
    // Cleanup the current scene if it exists and is different from target
    // This must complete successfully before we switch to the new scene
    if (oldIndex < scenes.size() && scenes[oldIndex]) {
        try {
            scenes[oldIndex]->cleanup();
        } catch (const std::exception& e) {
            std::cerr << "Error: Exception during scene cleanup: " << e.what() << '\n';
            // Don't proceed if cleanup fails - this could leave us in a bad state
            return;
        } catch (...) {
            std::cerr << "Error: Unknown exception during scene cleanup\n";
            return;
        }
    }
    
    // Update index AFTER cleanup completes successfully
    currentSceneIndex = index;
    
    // Initialize the new scene
    // Scenes should handle being initialized multiple times (they check isInitialized)
    if (scenes[currentSceneIndex]) {
        try {
            scenes[currentSceneIndex]->initialize();
        } catch (const std::exception& e) {
            std::cerr << "Error: Exception during scene initialization: " << e.what() << '\n';
            // If initialization fails, we're in a bad state
            // Reset to a safe state (first scene if available)
            if (!scenes.empty()) {
                currentSceneIndex = 0;
                if (scenes[0]) {
                    try {
                        scenes[0]->initialize();
                    } catch (...) {
                        // If even this fails, we're really in trouble
                        std::cerr << "Fatal: Cannot initialize any scene\n";
                    }
                }
            }
        } catch (...) {
            std::cerr << "Error: Unknown exception during scene initialization\n";
            // Same recovery as above
            if (!scenes.empty()) {
                currentSceneIndex = 0;
                if (scenes[0]) {
                    try {
                        scenes[0]->initialize();
                    } catch (...) {
                        std::cerr << "Fatal: Cannot initialize any scene\n";
                    }
                }
            }
        }
    }
}

} // namespace project

