#include "AssetViewWindow.h"

#include "Dialogs.h"
#include "EditorState.h"

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

		for (int i=0; i<fileMetadataTable.size(); i++)
		{
			ImGui::TableNextColumn();

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

				
				UUID uuid = (rel / filename).generic_string();
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
					UUID uuid = (rel / fMetadata.filename).generic_string();
					const AssetInfo& aInfo = assets->getAsset(uuid);
					ImGui::TextUnformatted(aInfo.name.c_str());
					ImGui::Separator();
					ImGui::Text("Type: %s", getAssetTypeAsStr(aInfo.aType).c_str());
					ImGui::Text("Path: %s", aInfo.filePath.c_str());
					ImGui::Text("Extension: %s", aInfo.ext.c_str());
					ImGui::Text("UUID: %s", aInfo.uuid.c_str());
					ImGui::Text("Size: (%.1f KB)", fMetadata.fileSize / 1024.0f);
					for (const auto& [attribName, attribVal] : aInfo.attributes)
					{
						ImGui::Text("%s: %s", attribName.c_str(), attribVal.c_str());

					}
					

				}
				ImGui::EndTooltip();
			}

			if (ImGui::BeginPopupContextItem("AssetContextMenu"))
			{
				if (ImGui::Selectable("Open"))
				{
					UUID uuid = (rel / fMetadata.filename).generic_string();
					const AssetInfo& aInfo = assets->getAsset(uuid);

					if (aInfo.aType == AssetType::MATERIAL)
					{
						ResourceWrapper<Material> mat = aInfo.data.as<Material>();
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
				}

				if (ImGui::Selectable("Rename"))
				{
					logDebug("Not yet implemented");
				}

				if (ImGui::Selectable("Delete"))
				{
					logDebug("Not yet implemented");
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
					UUID uuid = (rel / fMetadata.filename).generic_string();
					const AssetInfo& aInfo = assets->getAsset(uuid);

					if (aInfo.aType == AssetType::PREFAB && ImGui::Selectable("Instansiate"))
					{
						ResourceWrapper<Prefab> prefab = aInfo.data.as<Prefab>();
						if (prefab.isEmpty())
						{
							logWarning("Failed to cast asset to prefab asset.");
							continue;
						}
						prefab->Instansiate();
						updateScene();

					}
				}

				

				ImGui::EndPopup();
			}

			ImGui::PopID();
		}
		
		ImGui::EndTable();
	}
	ImGui::EndChild();
}