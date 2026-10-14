#include "MaterialThumbnailGenerator.h"

#include "geometry/Model.h"
#include "geometry/Mesh.h"
#include "render/Material.h"
#include "memory/BuiltInAssets.h"

bool MaterialThumbnailGenerator::generate(const AssetRecord& aInfo, const TextureResourceRef& target)
{
	MaterialAssetRef material(aInfo.uuid);
	if (material.isEmpty()) return false;

	// The preview renderer only has the PBR shader
	const MaterialRenderMode renderMode = material->getMaterialRenderMode();
	if (renderMode != MaterialRenderMode::Opaque && renderMode != MaterialRenderMode::Transparent)
		return false;

	MaterialResourceRef materialResource = material.resource();
	if (materialResource.isEmpty()) return false;

	ModelAssetRef sphere = BuiltInAssets::getByName<ModelAsset>(SGE_MESH_SPHERE);
	if (sphere.isEmpty()) return false;

	PreviewRenderDesc desc;
	desc.model = sphere.resource();
	if (desc.model.isEmpty()) return false;

	for (const auto& mesh : desc.model->getMeshes())
		desc.materials[mesh->getMaterialIndex()] = materialResource;

	desc.clearColor = glm::vec4(g_thumbnailBackground, 1.f);
	desc.flipY = true;

	return m_renderer.render(desc, target);
}
