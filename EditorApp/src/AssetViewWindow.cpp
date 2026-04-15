#include "AssetViewWindow.h"

#include "EditorState.h"
#include "imgui_internal.h"

#include <optional>
#include <unordered_set>
#include <vector>

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

	const int columns = 6; // number of thumbnails per row
	if (ImGui::BeginTable("FileBrowserGrid", columns))
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
					cwd.enterFolder(fMetadata.filename);
					s_assetViewLmbTracking = false;
					s_assetViewMarqueeDrag = false;
					earlyTableBreak = true;
					break;
				}
			}
			else
			{
				const std::string filename = fMetadata.filename;

				if (filename == "ProjectAssetRegistry")
					continue; // Skip unwanted files

				UUID uuid = getUIDFromFilename(cwd, filename);
				if (!assets->hasAsset(uuid)) continue;

				ImGui::BeginGroup();
				{

					const AssetRecord& aInfo = assets->getInfo(uuid);

					int iconID = 0;
					switch (aInfo.aType) {
					case AssetType::MODEL:     iconID = icons.at("model")->getID(); break;
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

			tileRects[i] = ImRect(p0, p1);

			ImGui::PushStyleColor(ImGuiCol_Header, IM_COL32(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_HeaderActive, IM_COL32(0, 0, 0, 0));

			ImGui::SetCursorScreenPos(p0);
			ImGui::InvisibleButton(("##tile_" + std::to_string(i)).c_str(), size);

			ImGui::PopStyleColor(3);

			if (ImGui::IsItemHovered() || s_assetViewSelectedIndices.count(i)) {
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
					UUID uuid = getUIDFromFilename(cwd, fMetadata.filename);
					const AssetRecord& aInfo = assets->getInfo(uuid);
					ImGui::TextUnformatted(aInfo.name.c_str());
					ImGui::Separator();
					ImGui::Text("Type: %s", getAssetTypeAsStr(aInfo.aType).c_str());
					ImGui::Text("Path: %s", aInfo.getScopedPath().scoped().string().c_str());
					ImGui::Text("UUID: %s", aInfo.uuid.str().c_str());
					ImGui::Text("Size: (%.1f KB)", fMetadata.fileSize / 1024.0f);				

				}
				ImGui::EndTooltip();
			}

			ImGui::TableNextColumn();
			ImGui::PopID();
		}
		
		ImGui::EndTable();

		if (!earlyTableBreak)
		{
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
				}
			}

			if (ImGui::BeginPopupContextWindow("AssetViewContext"))
			{
				const size_t selCount = s_assetViewSelectedIndices.size();
				if (selCount == 0)
				{
					if (ImGui::Selectable("Move here"))
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
							MaterialAssetRef mat = MaterialAssetRef(uuid);
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
							Engine::get()->getSubSystem<Assets>()->getAsset(uuid).reimportAsset();
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
						}
						else
						{
							// TODO implement directory recursive delete
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
					}
					else if (!io.KeyCtrl)
					{
						s_assetViewSelectedIndices.clear();
					}
				}
				s_assetViewLmbTracking = false;
				s_assetViewMarqueeDrag = false;
			}
		}
	}
	ImGui::EndChild();
}