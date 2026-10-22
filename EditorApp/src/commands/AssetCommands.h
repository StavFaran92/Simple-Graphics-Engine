#pragma once

#include "Command.h"
#include "AssetSnapshot.h"
#include "memory/AssetDescriptors.h"
#include "memory/AssetRef.h"

#include <memory>
#include <string>

class Asset;

// An asset that was just created by existing code; undo deletes it (and its file), redo creates it again
// from the build descriptor it was created with, under the same uuid so later steps still find it
class CreateAssetCmd : public Command
{
public:
	CreateAssetCmd(AssetRef<Asset> created, std::unique_ptr<ResourceBuildDescriptor> buildDesc, std::string label);

	bool execute(std::string& error) override;
	void undo() override;
	std::string label() const override { return m_label; }

private:
	UUID m_asset;
	AssetBuildDescriptor m_desc;
	std::unique_ptr<ResourceBuildDescriptor> m_buildDesc;
	std::string m_label;
};

// A change to one asset's data; not tied to a scene, so it applies whichever scene is open
class EditAssetCmd : public Command
{
public:
	EditAssetCmd(AssetSnapshot before, AssetSnapshot after, std::string label);

	bool execute(std::string& error) override;
	void undo() override;
	std::string label() const override { return m_label; }

private:
	AssetSnapshot m_before;
	AssetSnapshot m_after;
	std::string m_label;
};
