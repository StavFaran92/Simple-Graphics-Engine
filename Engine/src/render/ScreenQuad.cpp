#include "render/ScreenQuad.h"

#include "render/Vertex.h"
#include "geometry/Mesh.h"
#include "geometry/Quad.h"
#include "geometry/ShapeFactory.h"
#include "runtime/Entity.h"
#include "runtime/Scene.h"
#include "component/Component.h"

#include "component/RenderableComponent.h"
#include "component/ObjectComponent.h"

Entity ScreenQuad::GenerateScreenQuad(SGE_Regsitry* registry)
{
	auto entity = ShapeFactory::createQuad(registry);
	entity.RemoveComponent<RenderableComponent>();
	entity.RemoveComponent<ObjectComponent>();
	return entity;
}

//void ScreenQuad::draw(IRenderer& renderer, Shader* shader)
//{
//	Shader* currShader = (Shader*)m_shader.get();
//
//	if (shader)
//		currShader = shader;
//
//	renderer.SetDrawType(Renderer::DrawType::Triangles);
//
//	for (auto i = 0; i < m_meshes.size(); i++)
//	{
//		m_meshes[i]->render(*currShader, renderer);
//	}
//}