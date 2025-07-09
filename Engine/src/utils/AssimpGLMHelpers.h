#pragma once

#include "glm/glm.hpp"
#include<glm/gtx/quaternion.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class AssimpGLMHelpers
{
public:
	inline static glm::mat4 convertMat4ToGLMFormat(const aiMatrix4x4& assimpMat4)
	{
        glm::mat4 glmMat4;

        glmMat4[0][0] = assimpMat4.a1; // Column 0, Row 0
        glmMat4[0][1] = assimpMat4.b1; // Column 0, Row 1
        glmMat4[0][2] = assimpMat4.c1; // Column 0, Row 2
        glmMat4[0][3] = assimpMat4.d1; // Column 0, Row 3

        glmMat4[1][0] = assimpMat4.a2; // Column 1, Row 0
        glmMat4[1][1] = assimpMat4.b2; // Column 1, Row 1
        glmMat4[1][2] = assimpMat4.c2; // Column 1, Row 2
        glmMat4[1][3] = assimpMat4.d2; // Column 1, Row 3

        glmMat4[2][0] = assimpMat4.a3; // Column 2, Row 0
        glmMat4[2][1] = assimpMat4.b3; // Column 2, Row 1
        glmMat4[2][2] = assimpMat4.c3; // Column 2, Row 2
        glmMat4[2][3] = assimpMat4.d3; // Column 2, Row 3

        glmMat4[3][0] = assimpMat4.a4; // Column 3, Row 0
        glmMat4[3][1] = assimpMat4.b4; // Column 3, Row 1
        glmMat4[3][2] = assimpMat4.c4; // Column 3, Row 2
        glmMat4[3][3] = assimpMat4.d4; // Column 3, Row 3

        return glmMat4;
	}

    inline static glm::vec3 convertVec3ToGLMFormat(const aiVector3D& vec3)
    {
        return glm::vec3(vec3.x, vec3.y, vec3.z);
    }

    inline static glm::quat convertQuatToGLMFormat(const aiQuaternion& quat)
    {
        return glm::quat(quat.w, quat.x, quat.y, quat.z);
    }
};