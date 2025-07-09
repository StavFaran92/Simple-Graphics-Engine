#include "Transformation.h"
#include<glm/gtx/quaternion.hpp>
#include "LinearAlgebraUtil.h"
#include "Logger.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/matrix_decompose.hpp"

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


	// PG(x) = Parent global transform
	// L(x) = local transform
	// I take my local mat, transform by parent inverse global mat -> PG(x)-1*L(x)
	// so after all OPs the render will have PG(x)-1*PG(x)*L(x) = L(x)
	// i decompose PG(x)-1*L(x) into parts and set each one
	// for opposite I take L(x) = PG(x)*L(x) to undo my changes.

	auto& pTransform = parent.getComponent<Transformation>();
	glm::mat4& L = getLocalTransformation();
	glm::mat4& PG = pTransform.getWorldTransformation();

	glm::mat4& finalMat = glm::inverse(PG) * L;

	glm::vec3 scale, translation, skew;
	glm::quat rotation;
	glm::vec4 perspective;

	bool success = glm::decompose(finalMat, scale, rotation, translation, skew, perspective);

	setLocalScale(scale);
	setLocalRotation(rotation);
	setLocalPosition(translation);

	m_parent = parent;
	pTransform.addChild(m_entity);
}

void Transformation::removeParent()
{
	if (m_parent.handlerID() == Entity::EmptyEntity.handlerID())
		return;

	auto& pTransform = m_parent.getComponent<Transformation>();
	glm::mat4& L = getLocalTransformation();
	glm::mat4& PG = pTransform.getWorldTransformation();

	glm::mat4& finalMat = PG * L;

	glm::vec3 scale, translation, skew;
	glm::quat rotation;
	glm::vec4 perspective;

	bool success = glm::decompose(finalMat, scale, rotation, translation, skew, perspective);

	setLocalScale(scale);
	setLocalRotation(rotation);
	setLocalPosition(translation);

	
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
	m_globalRotation = m_localRotation;
	m_globalScale = m_localScale;

	if (m_parent.valid())
	{
		auto& pTransform = m_parent.getComponent<Transformation>();
		m_modelMatrix = pTransform.getWorldTransformation() * m_modelMatrix;
		m_globalRotation = pTransform.getWorldRotation() * m_globalRotation;
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

void Transformation::setWorldScale(glm::vec3 scale)
{
	auto parent = m_entity.getParent();
	if (parent.valid())
	{
		auto& parentTransform = parent.getComponent<Transformation>();
		setLocalScale(scale / parentTransform.getWorldScale());
	}
	else
	{
		setLocalScale(scale);
	}
	m_isDirty = true;
}

glm::vec3 Transformation::getLocalPosition() const
{
	return m_localTranslation;
}

glm::vec3 Transformation::getWorldPosition() const
{
	return glm::vec3(m_modelMatrix[3][0], m_modelMatrix[3][1], m_modelMatrix[3][2]);
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

glm::mat4 Transformation::worldToLocal(const glm::mat4& mat) const
{
	if (getParent().valid())
	{
		auto parentWorld = getParent().getComponent<Transformation>().getWorldTransformation();
		return glm::inverse(parentWorld) * mat;
	}
	else
	{
		return mat;
	}
}