#pragma once

template<typename T>
class ResourceWrapper;

class Resource
{
public:
	Resource() = default;
	virtual ~Resource() = default;

	//ResourceWrapper<Resource> loadResource(const std::string& fileLocation);

	//template<typename T>
	//static ResourceWrapper<T> createNewResource(ResourceID id)
	//{
	//	return ResourceWrapper<T>(id);
	//}

	static ResourceID getNewResourceID()
	{
		return resourceCounter++;
	}

	inline static ResourceID resourceCounter = 1;

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