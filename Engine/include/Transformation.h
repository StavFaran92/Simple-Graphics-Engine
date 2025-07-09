#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "core/Core.h"

#include "runtime/Entity.h"
#include "component/Component.h"

class EngineAPI Transformation : public Component
{
public:
	Transformation() :
		m_entity(Entity::EmptyEntity),
		m_localTranslation(0, 0, 0),
		m_localRotation(1, 0, 0, 0),
		m_localScale(1, 1, 1),
		m_relativeRot(1.f)
	{
		m_root = m_entity;
		m_isDirty = true;
		update();
	}

	Transformation(const Entity& entity) :
		m_entity(entity),
		m_localTranslation(0, 0, 0),
		m_localRotation(1, 0, 0, 0),
		m_localScale(1, 1, 1),
		m_relativeRot(1.f)
	{
		m_root = entity;
		m_isDirty = true;
		update();
	}

	Transformation(const Entity& entity, glm::vec3 translation) :
		m_entity(entity),
		m_localTranslation(translation),
		m_localRotation(1, 0, 0, 0),
		m_localScale(1, 1, 1),
		m_relativeRot(1.f)
	{
		m_root = entity;
		m_isDirty = true;
		update();
	}

	Transformation(const Entity& entity, glm::vec3 translation, glm::quat rotation) :
		m_entity(entity),
		m_localTranslation(translation),
		m_localRotation(rotation),
		m_localScale(1, 1, 1),
		m_relativeRot(1.f)
	{
		m_root = entity;
		m_isDirty = true;
		update();
	}


	void setLocalPosition(glm::vec3 pos);
	void setWorldPosition(glm::vec3 pos);
	void setLocalRotation(float angle, glm::vec3 axis);
	void setLocalRotation(glm::quat quat);
	void setWorldRotation(glm::quat quat);
	void setLocalScale(glm::vec3 scale);
	void setWorldScale(glm::vec3 scale);

	glm::mat4 getWorldTransformation() const;
	glm::vec3 getLocalPosition() const;
	glm::vec3 getWorldPosition() const;
	glm::quat getLocalRotationQuat() const;
	glm::vec3 getLocalRotationVec3() const;
	glm::quat getWorldRotation() const;
	glm::vec3 getLocalScale() const;
	glm::vec3 getWorldScale() const;

	glm::vec3 getForward() const;
	glm::vec3 getUp() const;
	glm::vec3 getRight() const;

	void translate(float x, float y, float z);
	void translate(glm::vec3 translation);

	void scale(float x, float y, float z);
	void scale(glm::vec3 translation);

	void rotate(glm::vec3 eulers);
	void rotate(glm::vec3 axis, float angle);
	void rotateAround(glm::vec3 pivot, glm::vec3 axis, float angle);
	void rotateLerp(glm::vec3 axis, float angle, float t);
	void rotateAroundLerp(glm::vec3 pivot, glm::vec3 axis, float angle, float t);
	void getWorldTransformation(glm::mat4& mat);
	glm::mat4 getLocalTransformation() const;

	void setParent(Entity parent);
	void removeParent();
	Entity getParent() const;
	Entity getRoot() const;
	Entity setRoot(Entity root);

	void update();
	void forceUpdate();

	glm::mat4 worldToLocal(const glm::mat4& mat) const;

	std::unordered_map<entity_id, Entity> getChildren();

	template <class Archive>
	void serialize(Archive& archive) {
		archive(m_localTranslation, m_localRotation, m_localScale, m_parent, m_entity, m_root, m_children);
	}

private:
	void addChild(Entity entity);
	void removeChild(Entity entity);
	glm::mat4 calculateModelMatrix();
private:
	friend class Archiver;

	glm::vec3 m_localTranslation;
	glm::quat m_localRotation;
	glm::vec3 m_localScale;

	glm::quat m_globalRotation;
	glm::vec3 m_globalScale;

	glm::mat4 m_modelMatrix;

	glm::mat4 m_relativeRot;


	glm::mat4 m_rootTransformation{ 1.f };

	Entity m_parent = Entity::EmptyEntity;
	Entity m_entity = Entity::EmptyEntity;
	Entity m_root = Entity::EmptyEntity;
	std::unordered_map<entity_id, Entity> m_children{};

	bool m_isDirty = true;
};
