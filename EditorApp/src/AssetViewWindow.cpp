#include "AssetViewWindow.h"

void AssetViewWindow::display()
{
	auto assets = Engine::get()->getSubSystem<Assets>();

	ImGui::Begin("Asset View", nullptr, windowFlags);

	static std::filesystem::path cwd = Engine::get()->getProjectDirectory();

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

	// Start grid layout
	const float thumbnailSize = 64.0f;
	const float padding = 16.0f;
	const int columns = 6; // number of thumbnails per row

	ImGui::Columns(columns, nullptr, false); // begin columns

	int i = 0;
	for (const auto& entry : std::filesystem::directory_iterator(cwd))
	{
		ImGui::PushID(i);
		i++;

		const std::string filenameFull = entry.path().filename().string();

		std::string filename = entry.path().stem().string();

		if (entry.is_directory())
		{
			ImGui::BeginGroup(); // Begin entry group (icon + name + extra info)

			std::string dirName = "[Dir] " + filename;
			// Create a small icon

			unsigned int iconID = icons.at("folder")->getID();
			ImGui::Image((ImTextureID)iconID, ImVec2(32, 32));
			ImGui::SameLine();

			// Draw filename and small info
			ImGui::Text("%s", dirName.c_str());

			ImGui::EndGroup();

			ImGui::NextColumn(); // move to next grid slot

			//ImGui::Separator(); // nice line between items
		}



		// Double click to open
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
		{
			cwd /= entry.path().filename();
			break;
		}


		ImGui::PopID();
	}

	static int selectedIndex = -1;   // keep track of selected group

	for (const auto& entry : std::filesystem::directory_iterator(cwd))
	{
		ImGui::PushID(i);
		i++;

		const std::string filenameFull = entry.path().filename().string();

		if (filenameFull == "entities.json" || filenameFull == "ProjectAssetRegistry.json")
			continue; // Skip unwanted files

		std::string filename = entry.path().stem().string();

		if (entry.is_regular_file())
		{
			if (!assets->hasAsset(filename)) continue;

			ImVec2 p0 = ImGui::GetCursorScreenPos();

			ImGui::BeginGroup(); // Begin entry group (icon + name + extra info)

			const AssetInfo& aInfo = assets->getAsset(filename);

			int iconID = 0;
			if (aInfo.aType == AssetType::MESH)
			{
				iconID = icons.at("mesh")->getID();
			}
			else if (aInfo.aType == AssetType::TEXTURE)
			{
				iconID = icons.at("texture")->getID();
			}
			else if (aInfo.aType == AssetType::ANIMATION)
			{
				iconID = icons.at("animation")->getID();
			}
			else if (aInfo.aType == AssetType::SHADER)
			{
				iconID = icons.at("shader")->getID();
			}
			else if (aInfo.aType == AssetType::PREFAB)
			{
				iconID = icons.at("prefab")->getID();
			}
			else if (aInfo.aType == AssetType::MATERIAL)
			{
				iconID = icons.at("material")->getID();
			}

			// Create a small icon
			ImGui::Image((ImTextureID)iconID, ImVec2(32, 32));
			ImGui::SameLine();

			std::string assetName = "[" + getAssetTypeAsStr(aInfo.aType) + "] " + aInfo.name;

			// Draw filename and small info
			ImGui::Text("%s", assetName.c_str());

			auto fileSize = std::filesystem::file_size(entry);
			ImGui::SameLine();
			ImGui::TextDisabled("(%.1f KB)", fileSize / 1024.0f);

			ImGui::EndGroup();






			

			// 2) Grab the group bounds
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
















			if (ImGui::BeginPopupContextItem("AssetContextMenu"))
			{
				if (ImGui::Selectable("Delete")) 
				{ 
					logDebug("Not yet implemented");
				}
				if (aInfo.aType == AssetType::PREFAB && ImGui::Selectable("Instansiate")) 
				{ 
					Resource<Prefab> prefab = aInfo.data.as<Prefab>();
					if (prefab.isEmpty())
					{
						logWarning("Failed to cast asset to prefab asset.");
						continue;
					}
					prefab->Instansiate();
					updateScene();
				
				}

				ImGui::EndPopup();
			}

			//ImGui::Separator(); // nice line between items

			ImGui::NextColumn(); // move to next grid slot
		}

		ImGui::PopID();

		// Double click to open
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
		{

		}


	}
	ImGui::Columns(1); // end columns

	ImGui::EndChild(); // end scrollable region
}