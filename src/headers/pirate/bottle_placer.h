#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "bottle.h"

class BottlePlacer {
public:
    // Basic place function (x, y, z)
    static void placeBottle(float x, float y, float z, std::vector<pirategame::Bottle>& bottleList);

    // vec3 overload
    static void placeBottle(const glm::vec3& position, std::vector<pirategame::Bottle>& bottleList);

    // Helper: Place a horizontal row of bottles along X
    static void placeRow(float startX, float endX, float y, float z, int count, std::vector<pirategame::Bottle>& bottleList);

    // Helper: Place an arc of bottles
    static void placeArc(float startX, float endX, float height, float yBase, float z, int count, std::vector<pirategame::Bottle>& bottleList);

    // Output code to console for quick copy-pasting
    static void exportLayoutCode(const std::vector<pirategame::Bottle>& bottleList);
};