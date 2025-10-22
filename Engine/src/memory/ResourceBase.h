#pragma once

#include <string>

template<typename T>
class ResourceWrapper;

class ResourceBase
{
public:
	ResourceBase() = default;
	virtual ~ResourceBase() = default;

	//static ResourceWrapper<ResourceBase> import(const std::string& fileLocation, AssetInfo& params);

	////virtual Resource<Asset> load(AssetInfo aInfo) = 0;

	//// CRTP
	//template <typename T>
	//static ResourceWrapper<T> import(const std::string& fileLocation, AssetInfo& params) {
	//	return T::import(fileLocation, params);
	//}

	//template <typename T>
	//static ResourceWrapper<T> load(AssetInfo& aInfo) {
	//	return T::load(aInfo);
	//}

public:
	//AssetInfo m_assetInfo;
};