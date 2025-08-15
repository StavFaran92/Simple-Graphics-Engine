#pragma once

#include "core/Core.h"
#include "core/Registry.h"

using entity_id = entt::id_type;
struct NativeScriptComponent;

/**
 * @class Entity
 * @brief Class for handling entities in a scene
 *
 * This class provides an interface for creating, adding, getting, and removing components
 * for entities in a scene.
 */
class EngineAPI Entity
{
public:
    static Entity EmptyEntity;
    /// Default constructor
    Entity() = default;

    /**
     * @brief Constructs an entity belonging to a scene
     * @param scene Pointer to the scene the entity belongs to
     */
    //Entity(entt::entity e, std::shared_ptr<SGE_Regsitry> registry)
    //    : m_entity(e), m_registry(registry)
    //{}

    Entity(entt::entity e, SGE_Regsitry* registry)
        : m_entity(e), m_registry(registry)
    {}

    ~Entity() {};

    /**
     * @brief Adds a component to the entity by moving an existing instance
     * @tparam T Type of the component
     * @param componentInstance Pointer to the component instance
     * @return Reference to the added component
     * @warning The behavior of the existing componentInstance after the call is undefined
     */
    template<typename T>
    T& addComponent(T* componentInstance)
    {
        assert(valid() && "Invalid entity.");

        if constexpr (std::is_same_v<T, NativeScriptComponent>)
        {
            componentInstance->entity = Entity(m_entity, m_registry);
        }

        T& component = m_registry->getRegistry().emplace_or_replace<T>(m_entity, *componentInstance);

        m_components.insert(typeid(T).name());

        return component;
    }

    /**
     * @brief Constructs and adds a new component to the entity
     * @tparam T Type of the component
     * @tparam Args Types of arguments to forward to the constructor of the component
     * @param args Arguments to forward to the constructor of the component
     * @return Reference to the added component
     */
    template<typename T, typename... Args>
    T& addComponent(Args&&... args)
    {
        static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component.");

        assert(valid() && "Invalid entity.");

        T& component = m_registry->getRegistry().emplace_or_replace<T>(m_entity, std::forward<Args>(args)...);

        if constexpr (std::is_same_v<T, NativeScriptComponent>)
        {
            component.entity = Entity(m_entity, m_registry);
        }

        m_components.insert(typeid(T).name());

        return component;
    }

    /**
     * @brief Retrieves a component from the entity
     * @tparam T Type of the component
     * @return Reference to the requested component
     */
    template<typename T>
    T& getComponent() const
    {
        static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component.");

        if (!valid())
        {
            throw std::runtime_error("Invalid Entity specified.");
        }
        if (!m_registry->getRegistry().any_of<T>(m_entity))
        {
            throw std::runtime_error("Entity does not have component: TBD");

        }
        return m_registry->getRegistry().get<T>(m_entity);
    }

    template<typename T>
    T* tryGetComponent()
    {
        assert(valid() && "Invalid entity.");
        return m_registry->getRegistry().try_get<T>(m_entity);
    }

    template<typename T>
    T* tryGetComponentInParent(bool includeSelf = true)
    {
        assert(valid() && "Invalid entity.");
        if (includeSelf)
        {
            auto comp = tryGetComponent<T>();
            if (comp)
            {
                return comp;
            }
        }

        auto parent = getParent();
        if (parent.valid())
        {
            return parent.tryGetComponentInParent<T>(true);
        }

        return nullptr;
    }

    template<typename T>
    T& getComponentInParent(bool includeSelf = true)
    {
        assert(valid() && "Invalid entity.");
        if (includeSelf)
        {
            if (HasComponent<T>())
            {
                return getComponent<T>();
            }
        }

        auto parent = getParent();
        if (parent.valid())
        {
            return parent.getComponentInParent<T>(true);
        }

        assert(false && "Component not found in tree hierarchy.");

        throw std::runtime_error("Component not found in tree hierarchy.");
    }

    template<typename T>
    T& getComponentInChildren(bool includeSelf = true)
    {
        assert(valid() && "Invalid entity.");
        if (includeSelf)
        {
            if (HasComponent<T>())
            {
                return getComponent<T>();
            }
        }

        auto childrens = getChildren();
        for (auto [id, child] : childrens)
        {
            T* comp = child.tryGetComponent<T>();
            if (comp)
            {
                return *comp;
            }
        }

        assert(false && "Component not found in tree hierarchy.");

        throw std::runtime_error("Component not found in tree hierarchy.");
    }

    /**
     * @brief Checks if the entity has a specific component
     * @tparam T Type of the component
     * @return true if the entity has the component, false otherwise
     */
    template<typename T>
    bool HasComponent() const
    {
        assert(valid() && "Invalid entity.");
        return m_registry->getRegistry().any_of<T>(m_entity);
    }

    /**
     * @brief Removes a component from the entity
     * @tparam T Type of the component
     */
    template<typename T>
    void RemoveComponent()
    {
        assert(valid() && "Invalid entity.");
        m_registry->getRegistry().remove<T>(m_entity);

        m_components.erase(typeid(T).name());

    }

    void remove();

    //TODO fix this , 
    // In conjuction with set parent this will cause hard to debug issues.
    // make sure to fix it
    void setRoot(Entity e);
    void setParent(Entity entity);
    void removeParent();
    Entity getParent() const;
    void addChildren(Entity entity);
    void removeChildren(Entity entity);
    std::unordered_map<entity_id, Entity> getChildren();
    Entity getChildByName(const std::string& name);

    Entity getRoot() const;

    /**
     * @brief Checks if the entity is valid
     * @return true if the entity is valid, false otherwise
     */
    bool valid() const
    {
        return m_registry != nullptr && m_entity != entt::null && m_registry->getRegistry().valid(m_entity);
    }

    bool operator==(const Entity& other)
    {
        return m_registry == other.m_registry && m_entity == other.m_entity;
    }

    bool operator!=(const Entity& other)
    {
        return m_registry != other.m_registry || m_entity != other.m_entity;
    }

    inline entt::entity handler() const
    {
        return m_entity;
    }

    inline entity_id handlerID() const
    {
        return (entity_id)m_entity;
    }

    void setRegistry(SGE_Regsitry* registry)
    {
        m_registry = registry;
    }

    template <class Archive>
    void serialize(Archive& archive) {
        archive(m_entity);
    }

    

private:
    SGE_Regsitry* m_registry = nullptr;  ///< Scene the entity belongs to
    entt::entity m_entity{ entt::null };  ///< ENTT entity instance

    // TODO remove
    std::set<std::string> m_components;
};

