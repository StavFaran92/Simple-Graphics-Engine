#pragma once

#include <vector>
#include <map>

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include<glm/gtx/quaternion.hpp>

struct KeyPosition
{
    glm::vec3 position;
    float timeStamp;

    bool operator<(const KeyPosition& other) const
    {
        return this->timeStamp < other.timeStamp;
    }

    bool operator<(float other) const
    {
        return this->timeStamp < other;
    }
};

struct KeyRotation
{
    glm::quat orientation;
    float timeStamp;

    bool operator<(const KeyPosition& other) const
    {
        return this->timeStamp < other.timeStamp;
    }

    bool operator<(float other) const
    {
        return this->timeStamp < other;
    }
};

struct KeyScale
{
    glm::vec3 scale;
    float timeStamp;

    bool operator<(const KeyPosition& other) const
    {
        return this->timeStamp < other.timeStamp;
    }

    bool operator<(float other) const
    {
        return this->timeStamp < other;
    }
};

class Bone
{
public:
    Bone(const std::vector<KeyPosition>& positions, const std::vector<KeyRotation>& rotations, const std::vector<KeyScale>& scalings);
	void update(float animationTime);
	const glm::mat4& getLocalTransform() const;
	glm::mat4 interpolatePosition(float animationTime) const;
	glm::mat4 interpolateRotation(float animationTime) const;
	glm::mat4 interpolateScale(float animationTime) const;
private:
    float getProgress(float t1, float t2, float v) const;
private:

	glm::mat4 m_localTransform;

	std::vector<KeyPosition> m_positions;
	std::vector<KeyRotation> m_rotations;
	std::vector<KeyScale> m_scalings;
};