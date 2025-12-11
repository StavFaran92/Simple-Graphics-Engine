#pragma once

#include <vector>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "mikktspace.h"

struct MeshData;

struct MikkMeshContext {
    MeshData* mesh;
};

int getNumFaces(const SMikkTSpaceContext* context);

int getNumVertsOfFace(const SMikkTSpaceContext*, int);

void getPosition(const SMikkTSpaceContext* context, float outPos[3], int face, int vert);

void getNormal(const SMikkTSpaceContext* context, float outNormal[3], int face, int vert);

void getTexCoord(const SMikkTSpaceContext* context, float outUV[2], int face, int vert);

void setTSpaceBasic(const SMikkTSpaceContext* context, const float tangent[3], float sign, int face, int vert);