#include "headers/pirate/bottle_placer.h"
#include <iostream>
#include <algorithm>

void BottlePlacer::placeBottle(float x, float y, float z, std::vector<pirategame::Bottle>& bottleList) {
    bottleList.emplace_back(x, y);
}

void BottlePlacer::placeBottle(const glm::vec3& position, std::vector<pirategame::Bottle>& bottleList) {
    placeBottle(position.x, position.y, position.z, bottleList);
}

void BottlePlacer::placeRow(float startX, float endX, float y, float z, int count, std::vector<pirategame::Bottle>& bottleList) {
    if (count <= 1) {
        placeBottle(startX, y, z, bottleList);
        return;
    }

    float step = (endX - startX) / (count - 1);
    for (int i = 0; i < count; ++i) {
        float x = startX + (i * step);
        placeBottle(x, y, z, bottleList);
    }
}

void BottlePlacer::placeArc(float startX, float endX, float height, float yBase, float z, int count, std::vector<pirategame::Bottle>& bottleList) {
    if (count <= 0) return;
    if (count == 1) {
        placeBottle((startX + endX) * 0.5f, yBase + height, z, bottleList);
        return;
    }

    for (int i = 0; i < count; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(count - 1);
        float x = startX + t * (endX - startX);
        float y = yBase + (4.0f * height * t * (1.0f - t));
        placeBottle(x, y, z, bottleList);
    }
}

void BottlePlacer::exportLayoutCode(const std::vector<pirategame::Bottle>& bottleList) {
    std::cout << "\n// --- Generated Bottle Layout Code ---\n";
    for (const auto& bottle : bottleList) {
        std::cout << "BottlePlacer::placeBottle("
            << bottle.position.x << "f, "
            << bottle.position.y << "f, 0.0f, gBottles);\n";
    }
    std::cout << "// -------------------------------------\n\n";
}