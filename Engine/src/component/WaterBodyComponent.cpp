#include "WaterBodyComponent.h"

#include "Transformation.h"
#include "MeshRendererComponent.h"

WaterBodyComponent::WaterBodyComponent(Entity entity)
	: entity(entity)
{

}

std::shared_ptr<TextureSampler> WaterBodyComponent::getWaterNormalSampler()
{
	MeshRendererComponent& meshRenderer = entity.getComponentInChildren<MeshRendererComponent>(false); // todo fix this is unsafe (getComponentInChildren itself is)
	AssetWrapper<Material> mat = meshRenderer.getMaterialBySlot(0);
	std::shared_ptr<TextureSampler> sampler = mat.get()->getSampler("waterNormalSampler");
	return sampler;
}
