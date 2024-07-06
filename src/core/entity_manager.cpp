#include "entity_manager.hpp"
namespace emp {
    EntityManager::EntityManager()
    {
        // Initialize the queue with all possible entity IDs
        for (Entity entity = 0; entity < MAX_ENTITIES; ++entity)
        {
            m_available_entities.push(entity);
        }
    }

    Entity EntityManager::createEntity()
    {
        assert(m_living_entity_count < MAX_ENTITIES && "Too many entities in existence.");

        // Take an ID from the front of the queue
        Entity id = m_available_entities.front();
        m_available_entities.pop();
        ++m_living_entity_count;

        return id;
    }

    void EntityManager::destroyEntity(Entity entity)
    {
        assert(entity < MAX_ENTITIES && "Entity out of range.");

        // Invalidate the destroyed entity's signature
        m_signatures[entity].reset();

        // Put the destroyed ID at the back of the queue
        m_available_entities.push(entity);
        --m_living_entity_count;
    }

    void EntityManager::setSignature(Entity entity, Signature signature)
    {
        assert(entity < MAX_ENTITIES && "Entity out of range.");

        // Put this entity's signature into the array
        m_signatures[entity] = signature;
    }

    Signature EntityManager::getSignature(Entity entity) const
    {
        assert(entity < MAX_ENTITIES && "Entity out of range.");

        // Get this entity's signature from the array
        return m_signatures[entity];
    }
};
