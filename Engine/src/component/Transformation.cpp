#include "component/Transformation.h"
#include<glm/gtx/quaternion.hpp>
#include "utils/LinearAlgebraUtil.h"
#include "core/Logger.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/matrix_decompose.hpp"

#include "component/ComponentSerializer.h"

#include "component/Component.h"
#include "runtime/Scene.h"



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
	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), localTranslation);
	glm::mat4 rotationMatrix = glm::mat4_cast(localRotation);
	glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), localScale);

	return translationMatrix * rotationMatrix * scaleMatrix;
}

void Transformation::setParent(Entity newParent)
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

	auto& pTransform = newParent.getComponent<Transformation>();
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

	m_parent = newParent;
	pTransform.addChild(entity);
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

	
	pTransform.removeChild(entity);
	m_parent = Entity::EmptyEntity;
}

Entity Transformation::getParent() const
{
	return m_parent;
}

Entity Transformation::getRoot() const
{
	return root;
}

Entity Transformation::setRoot(Entity root)
{
	return root = root;
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
	m_globalRotation = localRotation;
	m_globalScale = localScale;

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
	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), localTranslation);
	glm::mat4 rotationMatrix = glm::mat4_cast(localRotation);
	glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), localScale);

	return translationMatrix * rotationMatrix * scaleMatrix;
}

std::unordered_map<entity_id, Entity> Transformation::getChildren()
{
	return m_children;
}

void Transformation::attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene& scene)
{
	if (auto tc = std::dynamic_pointer_cast<Transformation>(c))
	{
		auto& transform = entityHandler.addComponent<Transformation>(*tc);
		transform.entity.setRegistry(&scene.getRegistry());
		transform.root.setRegistry(&scene.getRegistry());
		transform.m_parent.setRegistry(&scene.getRegistry());

		for (auto [_, entity] : tc->getChildren())
		{
			Entity eChild(entity.handler(), &scene.getRegistry());
			transform.addChild(eChild);
		}
	}
}

void Transformation::setLocalPosition(glm::vec3 pos)
{
	localTranslation = pos;

	m_isDirty = true;
}
void Transformation::setWorldPosition(glm::vec3 pos)
{
	auto parent = entity.getParent();
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
	localRotation = quat;
	m_isDirty = true;
}
void Transformation::setWorldRotation(glm::quat quat)
{
	auto parent = entity.getParent();
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
	localScale = scale;
	m_isDirty = true;
}

void Transformation::setWorldScale(glm::vec3 scale)
{
	auto parent = entity.getParent();
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
	return localTranslation;
}

glm::vec3 Transformation::getWorldPosition() const
{
	return glm::vec3(m_modelMatrix[3][0], m_modelMatrix[3][1], m_modelMatrix[3][2]);
}

glm::quat Transformation::getLocalRotationQuat() const
{
	return localRotation;
}

glm::vec3 Transformation::getLocalRotationVec3() const
{
	return glm::eulerAngles(localRotation);
}

glm::quat Transformation::getWorldRotation() const
{
	return m_globalRotation;
}

glm::vec3 Transformation::getLocalScale() const
{
	return localScale;
}

glm::vec3 Transformation::getWorldScale() const
{
	return m_globalScale;
}

glm::vec3 Transformation::getForward() const
{
	return localRotation * glm::vec3(0, 0, -1);
}

glm::vec3 Transformation::getUp() const
{
	return localRotation * glm::vec3(0, 1, 0);
}

glm::vec3 Transformation::getRight() const
{
	return localRotation * glm::vec3(1,0,0);
}

void Transformation::translate(float x, float y, float z)
{
	localTranslation.x += x;
	localTranslation.y += y;
	localTranslation.z += z;

	m_isDirty = true;
}

void Transformation::translate(glm::vec3 translation)
{
	translate(translation.x, translation.y, translation.z);
}

void Transformation::scale(float x, float y, float z)
{
	localScale.x *= x;
	localScale.y *= y;
	localScale.z *= z;

	m_isDirty = true;
}

void Transformation::scale(glm::vec3 scaleFactor)
{
	scale(scaleFactor.x, scaleFactor.y, scaleFactor.z);
}

void Transformation::rotate(glm::vec3 eulers)
{
	localRotation = glm::quat(eulers) * localRotation;

	m_isDirty = true;
}

void Transformation::rotateLerp(glm::vec3 axis, float angle, float t)
{
	localRotation = glm::mix(localRotation, glm::angleAxis(degToRad(angle), axis) * localRotation, t);// *m_orientationLocal;

	m_isDirty = true;
}


void Transformation::rotate(glm::vec3 axis, float angle)
{
	localRotation = glm::angleAxis(degToRad(angle), axis) * localRotation;

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