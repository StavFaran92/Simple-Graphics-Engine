#include "AssetCard.h"

#include "Common.h"
#include "Thumbnails/ThumbnailCache.h"

#include <algorithm>

namespace {

constexpr float kCardThumbPadding = 8.0f;
constexpr float kCardTypeStripHeight = 2.0f;
constexpr float kCardTextPadding = 4.0f;
constexpr float kCardTextLineGap = 2.0f;
constexpr float kCardRounding = 4.0f;

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

} // namespace

ImVec2 AssetCard::getSize()
{
	const float textHeight = ImGui::GetTextLineHeight() * 2.0f + kCardTextLineGap;
	return ImVec2(Width, Width + kCardTypeStripHeight + textHeight + kCardTextPadding * 2.0f);
}

ImU32 AssetCard::getTypeColor(AssetType aType)
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

ImTextureID AssetCard::getThumbnail(const AssetRecord& aInfo)
{
	ImTextureID thumbnail = ThumbnailCache::Instance().get(aInfo);
	if (thumbnail)
		return thumbnail;

	// No generated thumbnail (yet), use the type icon
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

void AssetCard::draw(ImTextureID thumbnail, const std::string& name, const std::string& typeLabel, ImU32 stripColor, bool selected)
{
	const ImVec2 cardSize = getSize();
	const ImVec2 p0 = ImGui::GetCursorScreenPos();
	const ImVec2 p1(p0.x + cardSize.x, p0.y + cardSize.y);

	ImGui::InvisibleButton("##card", cardSize);
	const bool hovered = ImGui::IsItemHovered();

	ImDrawList* dl = ImGui::GetWindowDrawList();

	// Card body
	ImU32 bgColor = hovered ? IM_COL32(64, 64, 64, 255) : IM_COL32(46, 46, 46, 255);
	dl->AddRectFilled(p0, p1, bgColor, kCardRounding);

	// Thumbnail area
	const ImVec2 thumbMax(p1.x, p0.y + Width);
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

void AssetCard::draw(const AssetRecord& aInfo, bool selected)
{
	draw(getThumbnail(aInfo), aInfo.name, getAssetTypeAsStr(aInfo.aType), getTypeColor(aInfo.aType), selected);
}

void AssetCardGrid::begin()
{
	// Number of cards per row adapts to the available width
	const float availWidth = ImGui::GetContentRegionAvail().x;
	m_columns = std::max(1, (int)((availWidth + AssetCard::Spacing) / (AssetCard::Width + AssetCard::Spacing)));
	m_count = 0;

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(AssetCard::Spacing, AssetCard::Spacing));
}

void AssetCardGrid::next()
{
	if (m_count % m_columns != 0)
		ImGui::SameLine();
	m_count++;
}

void AssetCardGrid::end()
{
	ImGui::PopStyleVar();
}
