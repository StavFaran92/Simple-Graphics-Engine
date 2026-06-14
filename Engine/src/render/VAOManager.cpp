#include "render/VAOManager.h"

#include "core/Engine.h"

GigaVAO& VAOManager::getGigaVAO(Type type)
{
	if (type == Type::StaticGeometry) return m_staticGeometry;
	if (type == Type::SkinnedGeometry) return m_skinnedGeometry;

	throw std::exception("Invalid type specified!");
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
	
}
