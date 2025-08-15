#include "runtime/Entity.h"

#include "component/Transformation.h"
#include <iostream>

Entity Entity::EmptyEntity{};

void Entity::setParent(Entity entity)
{
    assert(valid() && "Invalid entity.");
    assert(entity->valid() && "Invalid parent entity.");
    assert(entity->HasComponent<Transformation>() && "Entity does not contain HierarchyComponent.");
    auto& transform = getComponent<Transformation>();
    transform.setParent(entity);
}

Entity Entity::getParent() const
{
    assert(HasComponent<HierarchyComponent>() && "Entity does not contain HierarchyComponent.");
    auto& transform = getComponent<Transformation>();
    return transform.getParent();
}

//// Specialized version for ScriptableEntity
//template<>
//ScriptableEntity& Entity::getComponent()
//{
//    assert(valid() && "Invalid entity.");
//    assert(m_scene->getRegistry().has<NativeScriptComponent>(m_entity) && "Component does not exist.");
//    auto script = m_scene->getRegistry().get<NativeScriptComponent>(m_entity).script;
//    assert(script && "No script available.");
//    return *script;
//}
//
//template<>
//NativeScriptComponent& Entity::addComponent<NativeScriptComponent>(NativeScriptComponent* nsc)
//{
//    assert(valid() && "Invalid entity.");
//    NativeScriptComponent& component = m_scene->getRegistry().emplace_or_replace<NativeScriptComponent>(m_entity, *nsc);
//    component.entity = Entity(m_entity, m_scene);
//
//    std::cout << "test \n";
//
//    m_components.insert(typeid(nsc).name());
//
//    return component;
//}

void Entity::removeParent()
{
    assert(HasComponent<HierarchyComponent>() && "Entity does not contain HierarchyComponent.");
    auto& transform = getComponent<Transformation>();
    return transform.removeParent();
}

std::unordered_map<entity_id, Entity> Entity::getChildren()
{
    assert(HasComponent<HierarchyComponent>() && "Entity does not contain HierarchyComponent.");
    auto& transform = getComponent<Transformation>();
    return transform.getChildren();
}

Entity Entity::getRoot() const
{
    assert(HasComponent<HierarchyComponent>() && "Entity does not contain HierarchyComponent.");
    auto& transform = getComponent<Transformation>();
    return transform.getRoot();
}

void Entity::setRoot(Entity e)
{
    assert(HasComponent<HierarchyComponent>() && "Entity does not contain HierarchyComponent.");
    auto& transform = getComponent<Transformation>();
    transform.setRoot(e);
}

Entity Entity::getChildByName(const std::string& name)
{
    auto& childrens = getChildren();
    for(auto& [eid, child] : childrens)
    {
        auto& obj = child.getComponent<ObjectComponent>();
        if (obj.name == name)
        {
            return child;
        }
    }

    logWarning("Could not find child with name {}", name);
    return Entity::EmptyEntity;
}

void Entity::remove()
{
    auto& trans = getComponent<Transformation>();
    trans.removeParent();
    for (auto [eid, child] : trans.getChildren())
    {
        child.remove();
    }

    m_registry->removeEntity(*this);
}