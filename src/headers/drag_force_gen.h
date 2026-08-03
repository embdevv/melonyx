#ifndef DRAG_FORCE_GEN_H
#define DRAG_FORCE_GEN_H

#include "force_gen.h"
#include "particle.h"
#include <glm/glm.hpp>

namespace melonyx {

    class DragForceGenerator : public ForceGenerator {
    private:
        float k1;
        float k2;

    public:
        DragForceGenerator(float k1_val, float k2_val) : k1(k1_val), k2(k2_val) {}

        virtual void UpdateForce(Particle* particle, float duration) override;
    };

} // namespace melonyx

#endif