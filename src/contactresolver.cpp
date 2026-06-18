#include "headers/contactresolver.h"

void ContactResolver::ResolveContacts(std::list<ParticleContact*>& contacts, float time)
{
	for (auto it = contacts.begin(); it != contacts.end(); ++it)
	{
		(*it)->Resolve(time);
	}
}