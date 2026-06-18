#include "headers/contactresolver.h"
#include <algorithm>

void ContactResolver::ResolveContacts(std::list<ParticleContact*>& contacts, float time)
{
    int maxIterations = (int)contacts.size() * 4;

    for (int iter = 0; iter < maxIterations; iter++)
    {
        // Find contact with most negative separating speed (most urgent)
        ParticleContact* worst = nullptr;
        float worstSpeed = 0.0f; // only care about negative (approaching)

        for (auto it = contacts.begin(); it != contacts.end(); ++it)
        {
            float sep = (*it)->GetSeparatingSpeed();
            if (sep < worstSpeed)
            {
                worstSpeed = sep;
                worst = *it;
            }
        }

        if (worst == nullptr) break; // nothing left to resolve

        worst->Resolve(time);
    }
}