#include "geometry/Mesh.h"

#include "geometry/MeshBuilder.h"
#include "core/Logger.h"
#include "render/VertexArrayObject.h"

#include <GL\glew.h>

#include "utils/MikkTSpaceImpl.h"

Mesh::Mesh()
{
}

size_t Mesh::getNumOfVertices() const
{
	return getPositions().size();
}

// Compute the normals of the mesh
void Mesh::calculateNormals()
{
	//m_normals = std::vector<glm::vec3>(getNbVertices(), Vector3(0, 0, 0));

	//// For each triangular face
	//for (uint i = 0; i < getNbFaces(); i++) {

	//	// Get the three vertices index of the current face
	//	uint v1 = getVertexIndexInFace(i, 0);
	//	uint v2 = getVertexIndexInFace(i, 1);
	//	uint v3 = getVertexIndexInFace(i, 2);

	//	assert(v1 < getNbVertices());
	//	assert(v2 < getNbVertices());
	//	assert(v3 < getNbVertices());

	//	// Compute the normal of the face
	//	Vector3 p = getVertex(v1);
	//	Vector3 q = getVertex(v2);
	//	Vector3 r = getVertex(v3);
	//	Vector3 normal = (q - p).cross(r - p).normalize();

	//	// Add the face surface normal to the sum of normals at
	//	// each vertex of the face
	//	mNormals[v1] += normal;
	//	mNormals[v2] += normal;
	//	mNormals[v3] += normal;
	//}

	//// Normalize the normal at each vertex
	//for (uint i = 0; i < getNbVertices(); i++) {
	//	mNormals[i] = mNormals[i].normalize();
	//}
}

void Mesh::setVertexLayout(VertexLayout layout)
{
	m_layout = layout;
}

VertexLayout Mesh::getVertexLayout()
{
	return m_layout;
}

VertexArrayObject& Mesh::getVAO()
{
	return m_vao;
}

const VertexArrayObject& Mesh::getVAO() const
{
	return m_vao;
}

AABB Mesh::getAABB() const
{
	return m_aabb;
}

int Mesh::getMaterialIndex() const
{
	return materialIndex;
}

void Mesh::setRestTransform(glm::mat4 transform)
{
	m_restTransform = transform;
}

glm::mat4 Mesh::getRestTransform() const
{
	return m_restTransform;
}


void Mesh::setName(const std::string& name)
{
	m_name = name;
}
std::string Mesh::getName() const
{
	return m_name;
}

Mesh::~Mesh()
{
	logDebug( __FUNCTION__ );
}