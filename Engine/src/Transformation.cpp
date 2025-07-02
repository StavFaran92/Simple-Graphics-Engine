#include "Transformation.h"
#include<glm/gtx/quaternion.hpp>
#include "LinearAlgebraUtil.h"
#include "Logger.h"

glm::mat4 Transformation::getWorldTransformation() const
{
	return m_modelMatrix;
}

void Transformation::getWorldTransformation(glm::mat4& mat)
{
	mat = getWorldTransformation();
}

glm::mat4 Transformation::getLocalTransformation() const
{
	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), m_localTranslation);
	glm::mat4 rotationMatrix = glm::mat4_cast(m_localRotation);
	glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), m_localScale);

	return translationMatrix * rotationMatrix * scaleMatrix;
}

void Transformation::setParent(Entity parent)
{
	if (m_parent != Entity::EmptyEntity)
	{
		removeParent();
	}

	auto& pTransform = parent.getComponent<Transformation>();
	//m_rootTransformation = glm::inverse(pTransform.getWorldTransformation());

	setLocalPosition(getLocalPosition() - pTransform.getWorldPosition());
	setLocalRotation(glm::inverse(pTransform.getLocalRotationQuat()) * m_localRotation);
	setLocalScale(getLocalScale() / pTransform.getWorldScale());

	m_parent = parent;
	pTransform.addChild(m_entity);
	//parent.addChildren(m_entity);
}

void Transformation::removeParent()
{
	if (m_parent.handlerID() == Entity::EmptyEntity.handlerID())
		return;

	//m_parent.removeChildren(m_entity);
	auto& pTransform = m_parent.getComponent<Transformation>();
	pTransform.removeChild(m_entity);

	m_parent = Entity::EmptyEntity;
}

Entity Transformation::getParent() const
{
	return m_parent;
}

Entity Transformation::getRoot() const
{
	return m_root;
}

Entity Transformation::setRoot(Entity root)
{
	return m_root = root;
}

void Transformation::update()
{
	if (m_isDirty)
	{
		forceUpdate();
		return;
	}

	for (auto& [_,child] : m_children)
	{
		child.getComponent<Transformation>().update();
	}
}

void Transformation::forceUpdate()
{
	m_modelMatrix = calculateModelMatrix();
	m_globalTranslation = m_localTranslation;
	m_globalRotation = m_localRotation;
	m_globalScale = m_localScale;

	if (m_parent.valid())
	{
		auto& pTransform = m_parent.getComponent<Transformation>();
		m_modelMatrix = pTransform.getWorldTransformation() * m_modelMatrix;
		m_globalTranslation *= pTransform.getWorldPosition();
		m_globalRotation *= pTransform.getWorldRotation();
		m_globalScale *= pTransform.getWorldScale();
	}

	m_isDirty = false;

	for (auto& [_, child] : m_children)
	{
		child.getComponent<Transformation>().forceUpdate();
	}
}

void Transformation::addChild(Entity entity)
{
	m_children[entity.handlerID()] = entity;
}

void Transformation::removeChild(Entity entity)
{
	m_children.erase(entity.handlerID());
}

glm::mat4 Transformation::calculateModelMatrix()
{
	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), m_localTranslation);
	glm::mat4 rotationMatrix = glm::mat4_cast(m_localRotation);
	glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), m_localScale);

	return translationMatrix * rotationMatrix * scaleMatrix;
}

std::unordered_map<entity_id, Entity> Transformation::getChildren()
{
	return m_children;
}

void Transformation::setLocalPosition(glm::vec3 pos)
{
	m_localTranslation = pos;

	m_isDirty = true;
}
void Transformation::setWorldPosition(glm::vec3 pos)
{
	auto parent = m_entity.getParent();
	if (parent.valid())
	{
		auto& parentTransform = parent.getComponent<Transformation>();
		setLocalPosition(pos - parentTransform.getWorldPosition());
	}
	else
	{
		setLocalPosition(pos);
	}
	m_isDirty = true;
}
void Transformation::setLocalRotation(float angle, glm::vec3 axis)
{
	throw std::runtime_error("Not yet implemented");
	//m_change = true;
}
void Transformation::setLocalRotation(glm::quat quat)
{
	m_localRotation = quat;
	m_isDirty = true;
}
void Transformation::setWorldRotation(glm::quat quat)
{
	auto parent = m_entity.getParent();
	if (parent.valid())
	{
		auto& parentTransform = parent.getComponent<Transformation>();
		setLocalRotation(glm::inverse(parentTransform.getWorldRotation()) * quat);
	}
	else
	{
		setLocalRotation(quat);
	}
	m_isDirty = true;
}
void Transformation::setLocalScale(glm::vec3 scale)
{
	m_localScale = scale;
	m_isDirty = true;
}

glm::vec3 Transformation::getLocalPosition() const
{
	return m_localTranslation;
}

glm::vec3 Transformation::getWorldPosition() const
{
	return m_globalTranslation;
}

glm::quat Transformation::getLocalRotationQuat() const
{
	return m_localRotation;
}

glm::vec3 Transformation::getLocalRotationVec3() const
{
	return glm::eulerAngles(m_localRotation);
}

glm::quat Transformation::getWorldRotation() const
{
	return m_globalRotation;
}

glm::vec3 Transformation::getLocalScale() const
{
	return m_localScale;
}

glm::vec3 Transformation::getWorldScale() const
{
	return m_globalScale;
}

glm::vec3 Transformation::getForward() const
{
	return m_localRotation * glm::vec3(0, 0, -1);
}

glm::vec3 Transformation::getUp() const
{
	return m_localRotation * glm::vec3(0, 1, 0);
}

glm::vec3 Transformation::getRight() const
{
	return m_localRotation * glm::vec3(1,0,0);
}

void Transformation::translate(float x, float y, float z)
{
	m_localTranslation.x += x;
	m_localTranslation.y += y;
	m_localTranslation.z += z;

	m_isDirty = true;
}

void Transformation::translate(glm::vec3 translation)
{
	translate(translation.x, translation.y, translation.z);
}

void Transformation::scale(float x, float y, float z)
{
	m_localScale.x *= x;
	m_localScale.y *= y;
	m_localScale.z *= z;

	m_isDirty = true;
}

void Transformation::scale(glm::vec3 scaleFactor)
{
	scale(scaleFactor.x, scaleFactor.y, scaleFactor.z);
}

void Transformation::rotate(glm::vec3 eulers)
{
	m_localRotation = glm::quat(eulers) * m_localRotation;

	m_isDirty = true;
}

void Transformation::rotateLerp(glm::vec3 axis, float angle, float t)
{
	m_localRotation = glm::mix(m_localRotation, glm::angleAxis(degToRad(angle), axis) * m_localRotation, t);// *m_orientationLocal;

	m_isDirty = true;
}


void Transformation::rotate(glm::vec3 axis, float angle)
{
	m_localRotation = glm::angleAxis(degToRad(angle), axis) * m_localRotation;

	m_isDirty = true;
}

void Transformation::rotateAround(glm::vec3 pivot, glm::vec3 axis, float angle)
{
	auto p = glm::translate(glm::mat4(1.0f), pivot);
	auto pInv = glm::translate(glm::mat4(1.0f), -pivot);

	m_relativeRot = p * glm::mat4_cast(glm::angleAxis(degToRad(angle), axis)) * pInv * m_relativeRot;

	m_isDirty = true;
}


void Transformation::rotateAroundLerp(glm::vec3 pivot, glm::vec3 axis, float angle, float t)
{
	auto p = glm::translate(glm::mat4(1.0f), pivot);
	auto pInv = glm::translate(glm::mat4(1.0f), -pivot);

	m_relativeRot = p * glm::mat4_cast(glm::mix(glm::toQuat(m_relativeRot), glm::angleAxis(degToRad(angle), axis) * glm::toQuat(m_relativeRot), t)) * pInv;// *m_relativeRot;

	m_isDirty = true;
}