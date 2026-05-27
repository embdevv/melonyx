#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <glm/glm.hpp>
#include "headers/race_particle.h"

int main()
{
    using namespace melonyx;

    // Create particles: name, position, velocity, acceleration
    std::vector<RaceParticle> particles;
    particles.emplace_back("Red", glm::vec3(-350, -350, 201), 80.0f, 14.5f);
    particles.emplace_back("Green", glm::vec3(350, -350, 173), 90.0f, 8.0f);
    particles.emplace_back("Blue", glm::vec3(350, 350, -300), 130.0f, 1.0f);
    particles.emplace_back("Yellow", glm::vec3(-350, 350, -150), 110.0f, 3.0f);

    // Calculate results for all particles
    for (auto& particle : particles) {
        particle.CalculateRaceResults();
    }

    // Sort by time (ascending - fastest first)
    std::sort(particles.begin(), particles.end(),
              [](const RaceParticle& a, const RaceParticle& b) {
                  return a.timeToCenter < b.timeToCenter;
              });

    // Display results
    std::cout << "================================================================================\n";
    std::cout << "PARTICLE RACE SIMULATION - PROGRAMMING CHALLENGE 1\n";
    std::cout << "================================================================================\n\n";

    std::cout << std::fixed << std::setprecision(2);

    for (size_t rank = 0; rank < particles.size(); ++rank) {
        std::cout << "🏁 RANK #" << (rank + 1) << ": " << particles[rank].name << "\n";
        std::cout << "   Time to finish:          " << particles[rank].timeToCenter << " seconds\n";
        std::cout << "   Velocity at finish line: " << particles[rank].finalVelocity << " m/s\n";
        std::cout << "   Average velocity:        " << particles[rank].averageVelocity << " m/s\n";
        std::cout << "\n";
    }

    std::cout << "================================================================================\n";
    std::cout << "FINAL RANKING\n";
    std::cout << "================================================================================\n";

    for (size_t rank = 0; rank < particles.size(); ++rank) {
        std::cout << (rank + 1) << ". " << std::setw(8) << std::left << particles[rank].name
                  << " | Time: " << std::setw(7) << particles[rank].timeToCenter << "s | "
                  << "Finish Velocity: " << std::setw(7) << particles[rank].finalVelocity << " m/s | "
                  << "Avg Velocity: " << std::setw(7) << particles[rank].averageVelocity << " m/s\n";
    }

    std::cout << "================================================================================\n";

    return 0;
}
