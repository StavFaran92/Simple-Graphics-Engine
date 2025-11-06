#include "AssetViewWindow.h"

#include "Dialogs.h"
#include "EditorState.h"

#ifndef _WIN32
#include <unistd.h>
#endif

void openInVSCode(const std::string& path)
{
#ifndef _WIN32
	if (fork() == 0) { // child process
		execlp("code", "code", path.c_str(), (char*)nullptr);
		_exit(0); // exit child if exec fails
	}
#else
	// fallback for Windows
	std::thread([path]() {
		std::string command = "code \"" + path + "\"";
		std::system(command.c_str());
		}).detach();
#endif
}

void showInExplorer(const std::filesystem::path& p)
{
	auto absPath = std::filesystem::absolute(p);

	std::string command = "explorer /select,\"" + absPath.string() + "\"";
	system(command.c_str());
}

void AssetViewWindow::display()
{
	auto assets = Engine::get()->getSubSystem<Assets>();

	ImGui::Begin("Asset View", nullptr, windowFlags);

	static std::filesystem::path cwd = Engine::get()->getProjectDirectory();
	std::filesystem::path rel = std::filesystem::relative(cwd, Engine::get()->getProjectDirectory());
	if (rel == ".") rel = "";

	bool canGoBack = cwd != Engine::get()->getProjectDirectory();

	if (!canGoBack)
	{
		// Make button look disabled
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f); // 50% transparency
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)); // Gray color
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
	}

	// Back button
	bool clicked = ImGui::Button("<-", ImVec2(30, 30));

	// Restore style if it was pushed
	if (!canGoBack)
	{
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();
	}

	// Only handle click if it's allowed
	if (clicked && canGoBack)
	{
		cwd = cwd.parent_path();
	}


	ImGui::SameLine();

	// Header (fixed at top)
	ImGui::SetWindowFontScale(1.3f);
	ImGui::Text("%s", cwd.string().c_str());
	ImGui::SetWindowFontScale(1.0f);
	ImGui::Separator();

	// Scrollable region
	ImGui::BeginChild("FileBrowserScrollingRegion", ImVec2(0, 0), false);

	const int columns = 6; // number of thumbnails per row
	if (ImGui::BeginTable("FileBrowserGrid", columns))
	{
		static int selectedIndex = -1;   // keep track of selected group


		struct FileMetadata
		{
			std::string filename;
			std::string filePath;
			uintmax_t fileSize = 0;
			bool isDirectory = false;

		};
		std::vector<FileMetadata> fileMetadataTable;

		for (const auto& entry : std::filesystem::directory_iterator(cwd))
		{
			FileMetadata fMetadata;
			fMetadata.filename = entry.path().filename().string();

			if (fMetadata.filename == "entities.json" || fMetadata.filename == "ProjectAssetRegistry.json")
				continue; // Skip unwanted files

			fMetadata.filePath = entry.path().string();
			fMetadata.isDirectory = entry.is_directory();
			fMetadata.fileSize = entry.file_size();
			fileMetadataTable.push_back(fMetadata);
		}

		ImGui::TableNextColumn();

		for (int i=0; i<fileMetadataTable.size(); i++)
		{
			const FileMetadata& fMetadata = fileMetadataTable[i];
			ImGui::PushID(i);

			ImVec2 p0 = ImGui::GetCursorScreenPos();

			if (fMetadata.isDirectory)
			{
				ImGui::BeginGroup(); // Begin entry group (icon + name + extra info)
				{
					std::string dirName = fMetadata.filename;

					unsigned int iconID = icons.at("folder")->getID();
					ImGui::Image((ImTextureID)iconID, ImVec2(32, 32));
					ImGui::SameLine();

					ImGui::Text("%s", dirName.c_str());
				}
				ImGui::EndGroup();

				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
				{
					cwd /= fMetadata.filename;
					break;
				}
			}
			else
			{
				const std::string filename = fMetadata.filename;

				if (filename == "entities" || filename == "ProjectAssetRegistry")
					continue; // Skip unwanted files

				
				std::string relativeFilePath = (rel / filename).generic_string();
				UUID uuid = Engine::get()->getSubSystem<Assets>()->getAssetFromPath(relativeFilePath);
				if (!assets->hasAsset(uuid)) continue;

				ImGui::BeginGroup();
				{

					const AssetInfo& aInfo = assets->getAsset(uuid);

					int iconID = 0;
					switch (aInfo.aType) {
					case AssetType::MESH:     iconID = icons.at("mesh")->getID(); break;
					case AssetType::TEXTURE:  iconID = icons.at("texture")->getID(); break;
					case AssetType::ANIMATION:iconID = icons.at("animation")->getID(); break;
					case AssetType::SHADER:   iconID = icons.at("shader")->getID(); break;
					case AssetType::PREFAB:   iconID = icons.at("prefab")->getID(); break;
					case AssetType::MATERIAL: iconID = icons.at("material")->getID(); break;
					case AssetType::LUA_SCRIPT: iconID = icons.at("lua_script")->getID(); break;
					}

					// Create a small icon
					ImGui::Image((ImTextureID)iconID, ImVec2(32, 32));
					ImGui::SameLine();

					std::string assetName = aInfo.name;

					// Draw filename and small info
					ImGui::Text("%s", assetName.c_str());

				}
				ImGui::EndGroup();
			}

			ImVec2 p1 = ImGui::GetItemRectMax();
			ImVec2 size = ImGui::GetItemRectSize();

			ImGui::PushStyleColor(ImGuiCol_Header, IM_COL32(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_HeaderActive, IM_COL32(0, 0, 0, 0));

			// --- invisible selectable over the group ---
			ImGui::SetCursorScreenPos(p0);
			bool clicked = ImGui::Selectable(
				("##tile_" + std::to_string(i)).c_str(),
				selectedIndex == i,                          // show pressed state
				0,
				size
			);
			if (clicked) {
				selectedIndex = i;                           // mark as selected
			}

			ImGui::PopStyleColor(3);

			if (ImGui::IsItemHovered() || selectedIndex == i) {
				ImGui::GetWindowDrawList()->AddRect(
					p0, p1,
					ImGui::GetColorU32(ImGuiCol_HeaderHovered), // border color
					0.0f,                                       // rounding
					0,                                          // flags (0 = all corners)
					3.0f                                        // thickness
				);
			}

			// Restore cursor to where it would have been after the group
			ImGui::SetCursorScreenPos(p1);

			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				
				if (!fMetadata.isDirectory)
				{
					std::string relativeFilePath = (rel / fMetadata.filename).generic_string();
					UUID uuid = Engine::get()->getSubSystem<Assets>()->getAssetFromPath(relativeFilePath);
					const AssetInfo& aInfo = assets->getAsset(uuid);
					ImGui::TextUnformatted(aInfo.name.c_str());
					ImGui::Separator();
					ImGui::Text("Type: %s", getAssetTypeAsStr(aInfo.aType).c_str());
					ImGui::Text("Path: %s", aInfo.relativefilePath.c_str());
					ImGui::Text("Extension: %s", aInfo.ext.c_str());
					ImGui::Text("UUID: %s", aInfo.uuid.str().c_str());
					ImGui::Text("Size: (%.1f KB)", fMetadata.fileSize / 1024.0f);

					if (aInfo.attributes.size() > 0)
					{
						ImGui::LabelText("##Attributes:", "Attributes:");
						for (const auto& [attribName, attribVal] : aInfo.attributes)
						{
							ImGui::Text("%s: %s", attribName.c_str(), attribVal.c_str());

						}
					}
					

				}
				ImGui::EndTooltip();
			}

			if (ImGui::BeginPopupContextItem("AssetContextMenu"))
			{
				if (ImGui::Selectable("Open"))
				{
					std::string relativeFilePath = (rel / fMetadata.filename).generic_string();
					UUID uuid = Engine::get()->getSubSystem<Assets>()->getAssetFromPath(relativeFilePath);
					const AssetInfo& aInfo = assets->getAsset(uuid);

					if (aInfo.aType == AssetType::MATERIAL)
					{
						AssetWrapper<Material> mat = AssetWrapper<Material>(uuid);
						if (mat.isEmpty())
						{
							logError("Asset cast to material failed.");
						}
						else
						{
							EditorState::Instance().selectedMaterialForEdit = mat;
							EditorState::Instance().showMaterialEditWindow = true;

						}
					}

					else if (aInfo.aType == AssetType::LUA_SCRIPT)
					{
						openInVSCode(Engine::get()->getProjectDirectory() + "/" + aInfo.relativefilePath);
					}
				}

				if (ImGui::Selectable("Rename"))
				{
					logDebug("Not yet implemented");
				}

				if (ImGui::Selectable("Delete"))
				{
					std::string relativeFilePath = (rel / fMetadata.filename).generic_string();
					UUID uuid = Engine::get()->getSubSystem<Assets>()->getAssetFromPath(relativeFilePath);
					auto asset = Engine::get()->getSubSystem<Assets>()->getAsset(uuid);
					std::string path = asset.relativefilePath;
					Engine::get()->getSubSystem<Assets>()->deleteAsset(asset);
					std::filesystem::remove(Engine::get()->getProjectDirectory() + "/" + path);
				}

				if (ImGui::Selectable("Properties"))
				{
					logDebug("Not yet implemented");
				}

				if (ImGui::Selectable("Show in Explorer"))
				{

					showInExplorer(fMetadata.filePath);
				}

				ImGui::Separator();

				if (!fMetadata.isDirectory)
				{
					std::string relativeFilePath = (rel / fMetadata.filename).generic_string();
					UUID uuid = Engine::get()->getSubSystem<Assets>()->getAssetFromPath(relativeFilePath);
					const AssetInfo& aInfo = assets->getAsset(uuid);

					if (aInfo.aType == AssetType::PREFAB && ImGui::Selectable("Instansiate"))
					{
						AssetWrapper<Prefab> prefab = aInfo.data().as<Prefab>();
						if (prefab.isEmpty())
						{
							logWarning("Failed to cast asset to prefab asset.");
							continue;
						}
						prefab.resource()->Instansiate();
						updateScene();

					}
				}

				

				ImGui::EndPopup();
			}
			ImGui::TableNextColumn();
			ImGui::PopID();
		}
		
		ImGui::EndTable();
	}
	ImGui::EndChild();
}