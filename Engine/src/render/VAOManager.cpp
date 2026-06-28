#include "render/VAOManager.h"

#include "core/Engine.h"
#include "render/ElementBufferObject.h"
#include "render/VertexBufferObject.h"

GigaVAO& VAOManager::getGigaVAO(Type type)
{
	if (type == Type::StaticGeometry) return m_staticGeometry;
	if (type == Type::SkinnedGeometry) return m_skinnedGeometry;

	throw std::exception("Invalid type specified!");
}

VertexArrayObject& VAOManager::getQuadVAO()
{
	return m_quadVAO;
}

VertexArrayObject& VAOManager::getBoxVAO()
{
	return m_boxVAO;
}

VAOManager::VAOManager()
{
	Engine::get()->registerSubSystem<VAOManager>(this);

	VertexLayout staticGeometryLayout;
	staticGeometryLayout.attribs.push_back(LayoutAttribute::Positions);
	staticGeometryLayout.attribs.push_back(LayoutAttribute::Normals);
	staticGeometryLayout.attribs.push_back(LayoutAttribute::Texcoords);
	staticGeometryLayout.attribs.push_back(LayoutAttribute::Tangents);
	m_staticGeometry = GigaVAO(staticGeometryLayout);

	VertexLayout skinnedGeometryLayout;
	skinnedGeometryLayout.attribs.push_back(LayoutAttribute::Positions);
	skinnedGeometryLayout.attribs.push_back(LayoutAttribute::Normals);
	skinnedGeometryLayout.attribs.push_back(LayoutAttribute::Texcoords);
	skinnedGeometryLayout.attribs.push_back(LayoutAttribute::Tangents);
	skinnedGeometryLayout.attribs.push_back(LayoutAttribute::BoneIDs);
	skinnedGeometryLayout.attribs.push_back(LayoutAttribute::BoneWeights);
	m_skinnedGeometry = GigaVAO(skinnedGeometryLayout);

	static const float quadVerts[] = {
		-1.f,  1.f, 0.f,  0.f, 1.f,
		-1.f, -1.f, 0.f,  0.f, 0.f,
		 1.f, -1.f, 0.f,  1.f, 0.f,
		 1.f,  1.f, 0.f,  1.f, 1.f,
	};
	static unsigned int quadIndices[] = { 3, 2, 1, 1, 0, 3 };

	VertexLayout quadLayout;
	quadLayout.numOfVertices = 4;
	quadLayout.attribs.push_back(LayoutAttribute::Positions);
	quadLayout.attribs.push_back(LayoutAttribute::Texcoords);

	auto vbo = VertexBufferObject::createRaw(quadVerts, 4, sizeof(quadVerts), quadLayout);
	auto ebo = std::make_shared<ElementBufferObject>(quadIndices, 6);
	m_quadVAO.attachBuffer(vbo, ebo);
	m_quadVAO.build();
}
