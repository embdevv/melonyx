#pragma once
#include <list>
#include "particlecontact.h"

class ContactResolver
{
public:
	void ResolveContacts(std::list<ParticleContact*>& contacts, float time);
};