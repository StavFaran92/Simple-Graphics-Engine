#include "AssetViewWindow.h"

#include "EditorState.h"
#include "imgui_internal.h"

#include <optional>
#include <unordered_set>
#include <vector>
#include "menus/CreateMenu.h"

#ifndef _WIN32
#include <unistd.h>
#endif

std::set<UUID> g_moveAssetUIDList;

namespace {

std::unordered_set<int> s_assetViewSelectedIndices;
std::string s_assetViewLastCwd;

ImVec2 s_assetViewMarqueeMouseDownPos;
bool s_assetViewLmbTracking = false;
bool s_assetViewMarqueeDrag = false;

// Asset card layout (thumbnail on top, type strip, name + type label below)
constexpr float kCardWidth = 96.0f;
constexpr float kCardThumbPadding = 8.0f;
constexpr float kCardTypeStripHeight = 2.0f;
constexpr float kCardTextPadding = 4.0f;
constexpr float kCardTextLineGap = 2.0f;
constexpr float kCardSpacing = 8.0f;
constexpr float kCardRounding = 4.0f;

ImVec2 getCardSize()
{
	const float textHeight = ImGui::GetTextLineHeight() * 2.0f + kCardTextLineGap;
	return ImVec2(kCardWidth, kCardWidth + kCardTypeStripHeight + textHeight + kCardTextPadding * 2.0f);
}

ImU32 getAssetTypeColor(AssetType aType)
{
	switch (aType) {
	case AssetType::MODEL:      return IM_COL32(  0, 168, 242, 255);
	case AssetType::TEXTURE:    return IM_COL32(192,  64,  64, 255);
	case AssetType::ANIMATION:  return IM_COL32( 80, 200, 120, 255);
	case AssetType::SHADER:     return IM_COL32(170,  90, 230, 255);
	case AssetType::PREFAB:     return IM_COL32( 60, 110, 220, 255);
	case AssetType::MATERIAL:   return IM_COL32(110, 200,  60, 255);
	case AssetType::LUA_SCRIPT: return IM_COL32(230, 200,  60, 255);
	case AssetType::SCENE:      return IM_COL32(230, 130,  40, 255);
	default:                    return IM_COL32(128, 128, 128, 255);
	}
}

// For now every asset uses its type icon; real thumbnails will plug in here
ImTextureID getAssetThumbnail(const AssetRecord& aInfo)
{
	unsigned int iconID = 0;
	switch (aInfo.aType) {
	case AssetType::MODEL:      iconID = icons.at("model")->getID(); break;
	case AssetType::TEXTURE:    iconID = icons.at("texture")->getID(); break;
	case AssetType::ANIMATION:  iconID = icons.at("animation")->getID(); break;
	case AssetType::SHADER:     iconID = icons.at("shader")->getID(); break;
	case AssetType::PREFAB:     iconID = icons.at("prefab")->getID(); break;
	case AssetType::MATERIAL:   iconID = icons.at("material")->getID(); break;
	case AssetType::LUA_SCRIPT: iconID = icons.at("lua_script")->getID(); break;
	}
	return (ImTextureID)(intptr_t)iconID;
}

std::string ellipsizeText(const std::string& text, float maxWidth)
{
	if (ImGui::CalcTextSize(text.c_str()).x <= maxWidth)
		return text;

	const float ellipsisWidth = ImGui::CalcTextSize("...").x;
	std::string result = text;
	while (!result.empty() && ImGui::CalcTextSize(result.c_str()).x + ellipsisWidth > maxWidth)
		result.pop_back();
	return result + "...";
}

// Submits an invisible button the size of a card at the cursor and draws the card over it.
// The button is the last item, so IsItemHovered()/GetItemRect*() refer to the whole card.
void drawAssetCard(ImTextureID thumbnail, const std::string& name, const std::string& typeLabel, ImU32 stripColor, bool selected)
{
	const ImVec2 cardSize = getCardSize();
	const ImVec2 p0 = ImGui::GetCursorScreenPos();
	const ImVec2 p1(p0.x + cardSize.x, p0.y + cardSize.y);

	ImGui::InvisibleButton("##card", cardSize);
	const bool hovered = ImGui::IsItemHovered();

	ImDrawList* dl = ImGui::GetWindowDrawList();

	// Card body
	ImU32 bgColor = hovered ? IM_COL32(64, 64, 64, 255) : IM_COL32(46, 46, 46, 255);
	dl->AddRectFilled(p0, p1, bgColor, kCardRounding);

	// Thumbnail area
	const ImVec2 thumbMax(p1.x, p0.y + kCardWidth);
	dl->AddRectFilled(p0, thumbMax, IM_COL32(26, 26, 26, 255), kCardRounding, ImDrawFlags_RoundCornersTop);
	if (thumbnail)
	{
		dl->AddImage(thumbnail,
			ImVec2(p0.x + kCardThumbPadding, p0.y + kCardThumbPadding),
			ImVec2(thumbMax.x - kCardThumbPadding, thumbMax.y - kCardThumbPadding));
	}

	// Asset type strip
	if (stripColor != 0)
		dl->AddRectFilled(ImVec2(p0.x, thumbMax.y), ImVec2(p1.x, thumbMax.y + kCardTypeStripHeight), stripColor);

	// Name + type label
	const float textMaxWidth = cardSize.x - kCardTextPadding * 2.0f;
	ImVec2 textPos(p0.x + kCardTextPadding, thumbMax.y + kCardTypeStripHeight + kCardTextPadding);
	dl->AddText(textPos, ImGui::GetColorU32(ImGuiCol_Text), ellipsizeText(name, textMaxWidth).c_str());
	textPos.y += ImGui::GetTextLineHeight() + kCardTextLineGap;
	dl->AddText(textPos, ImGui::GetColorU32(ImGuiCol_TextDisabled), ellipsizeText(typeLabel, textMaxWidth).c_str());

	// Selection / hover outline
	if (selected)
		dl->AddRect(p0, p1, ImGui::GetColorU32(ImGuiCol_ButtonActive), kCardRounding, 0, 2.0f);
	else if (hovered)
		dl->AddRect(p0, p1, ImGui::GetColorU32(ImGuiCol_HeaderHovered), kCardRounding, 0, 1.0f);
}

} // namespace

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

UUID getUIDFromFilename(const WorkingDirectory cwd, const std::string& filename)
{
	ScopedPath considerPath = cwd.path();
	considerPath.setPath(filename);
	std::string considerScopedPathStr = considerPath.scoped().string();
	UUID uuid = Engine::get()->getSubSystem<Assets>()->getAssetFromPath(considerScopedPathStr).getUID();
	return uuid;
}

void AssetViewWindow::display()
{
	auto assets = Engine::get()->getSubSystem<Assets>();

	ImGui::Begin("Asset View", nullptr, windowFlags);

	WorkingDirectory& cwd = EditorState::Instance().getWorkingDir();
	if (cwd.string() != s_assetViewLastCwd)
	{
		s_assetViewSelectedIndices.clear();
		s_assetViewLastCwd = cwd.string();
		EditorState::Instance().clearAssetSelection();
	}
	//std::filesystem::path rel = std::filesystem::relative(cwd, Engine::get()->getProjectDirectory());
	//if (rel == ".") rel = "";

	//bool canGoBack = cwd != Engine::get()->getProjectDirectory();

	//if (!canGoBack)
	//{
	//	// Make button look disabled
	//	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f); // 50% transparency
	//	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)); // Gray color
	//	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
	//	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
	//}

	// Back button
	bool clicked = ImGui::Button("<-", ImVec2(30, 30));

	// Restore style if it was pushed
	//if (!canGoBack)
	//{
	//	ImGui::PopStyleColor(3);
	//	ImGui::PopStyleVar();
	//}

	// Only handle click if it's allowed
	if (clicked)
	{
		cwd.back();
	}


	ImGui::SameLine();

	// Header (fixed at top)
	ImGui::Text("%s", cwd.string().c_str());
	ImGui::Separator();

	// Scrollable region
	ImGui::BeginChild("FileBrowserScrollingRegion", ImVec2(0, 0), false);

	// Number of cards per row adapts to the window width
	const float availWidth = ImGui::GetContentRegionAvail().x;
	const int columns = std::max(1, (int)((availWidth + kCardSpacing) / (kCardWidth + kCardSpacing)));
	{
		struct FileMetadata
		{
			std::string filename;
			std::string filePath;
			uintmax_t fileSize = 0;
			bool isDirectory = false;

		};
		std::vector<FileMetadata> fileMetadataTable;

		auto iter = cwd.iter();
		for (const auto& entry : iter)
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

		std::vector<std::optional<ImRect>> tileRects(fileMetadataTable.size());
		bool earlyTableBreak = false;

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(kCardSpacing, kCardSpacing));
		int visibleCardCount = 0;

		for (int i=0; i<fileMetadataTable.size(); i++)
		{
			const FileMetadata& fMetadata = fileMetadataTable[i];

			ImTextureID thumbnail = nullptr;
			std::string cardName;
			std::string cardTypeLabel;
			ImU32 cardStripColor = 0;

			if (fMetadata.isDirectory)
			{
				thumbnail = (ImTextureID)(intptr_t)icons.at("folder")->getID();
				cardName = fMetadata.filename;
				cardTypeLabel = "Folder";
			}
			else
			{
				if (fMetadata.filename == "ProjectAssetRegistry")
					continue; // Skip unwanted files

				UUID uuid = getUIDFromFilename(cwd, fMetadata.filename);
				if (!assets->hasAsset(uuid)) continue;

				const AssetRecord& aInfo = assets->getInfo(uuid);
				thumbnail = getAssetThumbnail(aInfo);
				cardName = aInfo.name;
				cardTypeLabel = getAssetTypeAsStr(aInfo.aType);
				cardStripColor = getAssetTypeColor(aInfo.aType);
			}

			if (visibleCardCount % columns != 0)
				ImGui::SameLine();
			visibleCardCount++;

			ImGui::PushID(i);

			drawAssetCard(thumbnail, cardName, cardTypeLabel, cardStripColor, s_assetViewSelectedIndices.count(i) > 0);
			tileRects[i] = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());

			if (fMetadata.isDirectory && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
			{
				cwd.enterFolder(fMetadata.filename);
				s_assetViewLmbTracking = false;
				s_assetViewMarqueeDrag = false;
				earlyTableBreak = true;
				ImGui::PopID();
				break;
			}

			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				
				if (!fMetadata.isDirectory)
				{
					UUID uuid = getUIDFromFilename(cwd, fMetadata.filename);
					const AssetRecord& aInfo = assets->getInfo(uuid);
					ImGui::TextUnformatted(aInfo.name.c_str());
					ImGui::Separator();
					ImGui::Text("Type: %s", getAssetTypeAsStr(aInfo.aType).c_str());
					ImGui::Text("Path: %s", aInfo.getScopedPath().scoped().string().c_str());
					ImGui::Text("UUID: %s", aInfo.uuid.str().c_str());
					ImGui::Text("Size: (%.1f KB)", fMetadata.fileSize / 1024.0f);

				}
				else
				{
					ImGui::TextUnformatted(fMetadata.filename.c_str());
				}
				ImGui::EndTooltip();
			}

			ImGui::PopID();
		}

		ImGui::PopStyleVar();

		if (!earlyTableBreak)
		{
			auto syncCurrentAssetEdit = [&]() {
				EditorState::Instance().clearAssetSelection();

				if (s_assetViewSelectedIndices.size() != 1)
					return;

				const int selectedIndex = (int)*s_assetViewSelectedIndices.begin();
				if (selectedIndex < 0 || selectedIndex >= (int)fileMetadataTable.size())
					return;

				const FileMetadata& meta = fileMetadataTable[selectedIndex];
				if (meta.isDirectory)
					return;

				UUID uid = getUIDFromFilename(cwd, meta.filename);
				if (!assets->hasAsset(uid))
					return;

				AssetRef<Asset> asset = assets->getAsset(uid);
				EditorState::Instance().selectAssetForEdit(asset);

			};

			ImGuiIO& io = ImGui::GetIO();
			const bool childHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

			if (ImGui::IsMouseClicked(1) && childHovered)
			{
				int rmbHit = -1;
				for (int ti = 0; ti < (int)tileRects.size(); ++ti)
				{
					if (!tileRects[ti].has_value())
						continue;
					if (tileRects[ti]->Contains(io.MousePos))
						rmbHit = ti;
				}
				if (rmbHit >= 0 && !s_assetViewSelectedIndices.count(rmbHit))
				{
					s_assetViewSelectedIndices.clear();
					s_assetViewSelectedIndices.insert(rmbHit);
					syncCurrentAssetEdit();
				}
			}

			if (ImGui::BeginPopupContextWindow("AssetViewContext"))
			{
				const size_t selCount = s_assetViewSelectedIndices.size();
				if (selCount == 0)
				{
					CreateMenu::display();

					if (ImGui::MenuItem("Move here")) 
					{
						for (auto uid : g_moveAssetUIDList)
						{
							if (!Engine::get()->getSubSystem<Assets>()->hasAsset(uid))
							{
								logWarning("Invalid asset specified.");
							}
							else
							{
								ScopedPath targetDirectory = cwd.path();
								Engine::get()->getSubSystem<Assets>()->moveAsset(uid, targetDirectory);
							}
						}
					}
				}
				else if (selCount == 1)
				{
					const int i = (int)*s_assetViewSelectedIndices.begin();
					const FileMetadata& fMetadata = fileMetadataTable[i];

					if (ImGui::Selectable("Edit"))
					{
						UUID uuid = getUIDFromFilename(cwd, fMetadata.filename);
						const AssetRecord& aInfo = assets->getInfo(uuid);

						if (aInfo.aType == AssetType::MATERIAL)
						{
							MaterialAssetRef mat(uuid);
							if (mat.isEmpty())
								logError("Asset cast to material failed.");
							else
							{
								EditorState::Instance().selectedMaterialForEdit = mat;
								EditorState::Instance().setState("MaterialEditDialog", true);
							}
						}
						else if (aInfo.aType == AssetType::LUA_SCRIPT)
							openInVSCode(aInfo.getAbsolutePath());
						else if (aInfo.aType == AssetType::SHADER)
							openInVSCode(aInfo.getAbsolutePath());
					}

					if (ImGui::Selectable("Reimport"))
					{
						if (!fMetadata.isDirectory)
						{
							UUID uuid = getUIDFromFilename(cwd, fMetadata.filename);
							Engine::get()->getSubSystem<Assets>()->reimportAsset(uuid);
						}
					}

					if (ImGui::Selectable("Rename"))
					{
						if (!fMetadata.isDirectory)
						{
							UUID uuid = getUIDFromFilename(cwd, fMetadata.filename);
							Engine::get()->getSubSystem<Assets>()->renameAsset(uuid, "test");
						}
					}

					if (ImGui::Selectable("Move"))
					{
						if (!fMetadata.isDirectory)
						{
							g_moveAssetUIDList.clear();
							g_moveAssetUIDList.insert(getUIDFromFilename(cwd, fMetadata.filename));
						}
					}

					if (ImGui::Selectable("Delete"))
					{
						if (!fMetadata.isDirectory)
						{
							UUID uuid = getUIDFromFilename(cwd, fMetadata.filename);
							auto& asset = Engine::get()->getSubSystem<Assets>()->getAsset(uuid);
							asset.erase();

							// Drop the selection so nothing keeps referencing the deleted asset
							s_assetViewSelectedIndices.clear();
							EditorState::Instance().clearAssetSelection();
						}
						else
						{
							// Collect assets first so we don't erase files while iterating the folder
							std::filesystem::path folderPath = fMetadata.filePath;
							std::vector<UUID> folderAssetUIDs;
							for (const auto& entry : std::filesystem::recursive_directory_iterator(folderPath))
							{
								if (!entry.is_regular_file())
									continue;

								std::filesystem::path relPath = std::filesystem::relative(entry.path(), folderPath.parent_path());
								UUID uuid = getUIDFromFilename(cwd, relPath.string());
								if (assets->hasAsset(uuid))
									folderAssetUIDs.push_back(uuid);
							}

							// Erase through the asset system so the registry stays in sync
							for (UUID uuid : folderAssetUIDs)
								assets->getAsset(uuid).erase();

							std::error_code ec;
							std::filesystem::remove_all(folderPath, ec);
							if (ec)
								logError("Failed to delete folder {}: {}", folderPath.string(), ec.message());

							// Drop the selection so nothing keeps referencing the deleted folder
							s_assetViewSelectedIndices.clear();
							EditorState::Instance().clearAssetSelection();
						}
					}

					if (ImGui::Selectable("Properties"))
						logDebug("Not yet implemented");

					if (ImGui::Selectable("Show in Explorer"))
						showInExplorer(fMetadata.filePath);

					ImGui::Separator();

					if (!fMetadata.isDirectory)
					{
						UUID uuid = getUIDFromFilename(cwd, fMetadata.filename);
						const AssetRecord& aInfo = assets->getInfo(uuid);

						if (aInfo.aType == AssetType::PREFAB && ImGui::Selectable("Instansiate"))
						{
							PrefabAssetRef prefab = assets->getAsset(uuid).as<PrefabAsset>();
							if (prefab.isEmpty())
								logWarning("Failed to cast asset to prefab asset.");
							else
							{
								prefab.resource()->Instansiate();
								updateScene();
							}
						}

						if (aInfo.aType == AssetType::SCENE && ImGui::Selectable("Open"))
						{
							SceneAssetRef scene = assets->getAsset(uuid).as<SceneAsset>();
							if (scene.isEmpty())
								logWarning("Failed to cast asset to scene asset.");
							else
							{
								Engine::get()->getContext()->setActiveScene(scene.resource()->getID());
								setupScene();
								updateScene();
							}
						}

						if (aInfo.aType == AssetType::SHADER && ImGui::Selectable("Recompile"))
						{
							ShaderAssetRef shader = assets->getAsset(uuid).as<ShaderAsset>();
							if (shader.isEmpty())
								logWarning("Failed to cast asset to shader asset.");
							else
								shader.resource()->recompile();
						}
					}
				}
				else
				{
					if (ImGui::Selectable("Move"))
					{
						g_moveAssetUIDList.clear();

						for(auto index : s_assetViewSelectedIndices)
						{
							const FileMetadata& meta = fileMetadataTable[index];
							if (meta.isDirectory)
								continue; // or handle dirs separately

							UUID uid = getUIDFromFilename(cwd, meta.filename);
							g_moveAssetUIDList.insert(uid);
						}
					}
				}

				ImGui::EndPopup();
			}

			if (ImGui::IsMouseClicked(0) && childHovered)
			{
				s_assetViewMarqueeMouseDownPos = io.MousePos;
				s_assetViewLmbTracking = true;
				s_assetViewMarqueeDrag = false;
			}

			if (s_assetViewLmbTracking && ImGui::IsMouseDragging(0, io.MouseDragThreshold))
				s_assetViewMarqueeDrag = true;

			if (s_assetViewMarqueeDrag)
			{
				ImGuiWindow* win = ImGui::GetCurrentWindow();
				const ImRect clipR = win->InnerClipRect;

				ImVec2 a = s_assetViewMarqueeMouseDownPos;
				ImVec2 b = io.MousePos;
				ImRect band(ImMin(a, b), ImMax(a, b));
				band.ClipWith(clipR);

				if (band.Min.x < band.Max.x && band.Min.y < band.Max.y)
				{
					ImDrawList* dl = ImGui::GetWindowDrawList();
					dl->PushClipRect(clipR.Min, clipR.Max, true);
					dl->AddRectFilled(band.Min, band.Max, IM_COL32(100, 150, 255, 40));
					dl->AddRect(band.Min, band.Max, IM_COL32(100, 150, 255, 200), 0.0f, 0, 1.0f);
					dl->PopClipRect();
				}
			}

			if (ImGui::IsMouseReleased(0) && s_assetViewLmbTracking)
			{
				const bool wasMarquee = s_assetViewMarqueeDrag;
				if (wasMarquee)
				{
					ImGuiWindow* win = ImGui::GetCurrentWindow();
					const ImRect clipR = win->InnerClipRect;

					ImVec2 a = s_assetViewMarqueeMouseDownPos;
					ImVec2 b = io.MousePos;
					ImRect band(ImMin(a, b), ImMax(a, b));
					band.ClipWith(clipR);

					std::unordered_set<int> picked;
					if (band.Min.x >= band.Max.x || band.Min.y >= band.Max.y)
					{
						if (!io.KeyCtrl)
							s_assetViewSelectedIndices.clear();
					}
					else
					{
						for (int ti = 0; ti < (int)tileRects.size(); ++ti)
						{
							if (!tileRects[ti].has_value())
								continue;
							if (band.Overlaps(*tileRects[ti]))
								picked.insert(ti);
						}
						if (io.KeyCtrl)
						{
							for (int id : picked)
								s_assetViewSelectedIndices.insert(id);
						}
						else
						{
							s_assetViewSelectedIndices = std::move(picked);
						}
					}
					syncCurrentAssetEdit();
				}
				else
				{
					ImVec2 mp = io.MousePos;
					int hit = -1;
					for (int ti = 0; ti < (int)tileRects.size(); ++ti)
					{
						if (!tileRects[ti].has_value())
							continue;
						if (tileRects[ti]->Contains(mp))
							hit = ti;
					}
					if (hit >= 0)
					{
						if (io.KeyCtrl)
						{
							if (s_assetViewSelectedIndices.count(hit))
								s_assetViewSelectedIndices.erase(hit);
							else
								s_assetViewSelectedIndices.insert(hit);
						}
						else
						{
							s_assetViewSelectedIndices.clear();
							s_assetViewSelectedIndices.insert(hit);
						}
						syncCurrentAssetEdit();
					}
					else if (!io.KeyCtrl)
					{
						s_assetViewSelectedIndices.clear();
						syncCurrentAssetEdit();
					}
				}
				s_assetViewLmbTracking = false;
				s_assetViewMarqueeDrag = false;
			}
		}
	}
	ImGui::EndChild();
}