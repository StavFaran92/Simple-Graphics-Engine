#include "WaterBodyComponent.h"

#include "Transformation.h"
#include "MeshRendererComponent.h"
#include "core/Logger.h"

std::vector<AssetRef<Asset>> WaterBodyComponent::gatherDependenciesInternal() const
{
	std::vector<AssetRef<Asset>> dependencies;

	if (waterBodyNormal)
	{
		dependencies.push_back(waterBodyNormal->texture);
	}
	return dependencies;
}
