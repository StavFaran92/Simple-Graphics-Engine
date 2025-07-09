#include "Bone.h"

template<typename T>
int getDescreteIndex(const std::vector<T>& list, float animationTime)
{
    auto it = std::lower_bound(list.begin(), list.end(), animationTime);
    return std::max(0, (int)std::distance(list.begin(), it) - 1);
}

Bone::Bone(const std::vector<KeyPosition>& positions, const std::vector<KeyRotation>& rotations, const std::vector<KeyScale>& scalings)
    : m_positions(positions), m_rotations(rotations), m_scalings(scalings)
{
}

float Bone::getProgress(float t1, float t2, float v) const
{
    return (v - t1) / (t2 - t1);
}

glm::mat4 Bone::interpolatePosition(float animationTime) const
{
    if (m_positions.size() == 1)
    {
        return glm::translate(glm::mat4(1.0f), m_positions[0].position);
    }

    int p0Index = getDescreteIndex(m_positions, animationTime);
    int p1Index = p0Index + 1;
    float progress = getProgress(m_positions[p0Index].timeStamp, m_positions[p1Index].timeStamp, animationTime);
    glm::vec3 finalPosition = glm::mix(m_positions[p0Index].position, m_positions[p1Index].position, progress);
    return glm::translate(glm::mat4(1.0f), finalPosition);
}

glm::mat4 Bone::interpolateRotation(float animationTime) const
{
    if (m_rotations.size() == 1)
    {
        auto rotation = glm::normalize(m_rotations[0].orientation);
        return glm::toMat4(rotation);
    }

    int p0Index = getDescreteIndex(m_rotations, animationTime);
    int p1Index = p0Index + 1;
    float progress = getProgress(m_rotations[p0Index].timeStamp, m_rotations[p1Index].timeStamp, animationTime);
    glm::quat finalRotation = glm::slerp(m_rotations[p0Index].orientation, m_rotations[p1Index].orientation, progress);
    auto rotation = glm::normalize(finalRotation);
    return glm::toMat4(rotation);;
}

glm::mat4 Bone::interpolateScale(float animationTime) const
{
    if (m_scalings.size() == 1)
    {
        return glm::scale(glm::mat4(1.0f), m_scalings[0].scale);
    }

    int p0Index = getDescreteIndex(m_scalings, animationTime);
    int p1Index = p0Index + 1;
    float progress = getProgress(m_scalings[p0Index].timeStamp, m_scalings[p1Index].timeStamp, animationTime);
    glm::vec3 finalScale = glm::mix(m_scalings[p0Index].scale, m_scalings[p1Index].scale, progress);
    return glm::scale(glm::mat4(1.0f), finalScale);
}

void Bone::update(float animationTime)
{
    auto& translation = interpolatePosition(animationTime);
    auto& rotation = interpolateRotation(animationTime);
    auto& scale = interpolateScale(animationTime);
    m_localTransform = translation * rotation * scale;
}

const glm::mat4& Bone::getLocalTransform() const
{
	return m_localTransform;
}
