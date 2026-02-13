#include "EditorState.h"

#include "Dialogs/DialogBase.h"

#include "FoliagePaintTool.h"
#include "TransformTool.h"
#include "TerrainDeformTool.h"

void EditorState::init()
{
    cwd.m_path = ScopedPath::ContentPath();
}

bool EditorState::getState(const std::string& state)
{
    if (auto it = m_states.find(state); it != m_states.end())
        return it->second;

    return false;
}

bool EditorState::setState(const std::string& state, bool enabled)
{
    bool old = getState(state);
    m_states[state] = enabled;
    return old;
}

void EditorState::addDialogDisplay(DialogBase* dialog)
{
    m_dialogs.push_back(dialog);
}

void EditorState::displayDialogs()
{
    for (auto dialog : m_dialogs)
    {
        dialog->display();
    }
}

void EditorState::setActiveEditorTool(EditorTool::Type toolType)
{
    // Deactive currently active tool
    if (m_activeEditorToolType != EditorTool::Type::None && m_activeEditorTool)
        m_activeEditorTool->onDeactivate();

    // Set new tool
    m_activeEditorToolType = toolType;
    m_activeEditorTool = getTool(toolType);

    // Activate new tool
    if (m_activeEditorTool)
        m_activeEditorTool->onActivate();
}

EditorTool::Type EditorState::getActiveToolType() const
{
    return m_activeEditorToolType;
}

std::shared_ptr<EditorTool> EditorState::getActiveEditorTool()
{
    return m_activeEditorTool;
}

WorkingDirectory& EditorState::getWorkingDir()
{
    return cwd;
}

std::shared_ptr<EditorTool> EditorState::getTool(EditorTool::Type type)
{
    static std::unordered_map<EditorTool::Type, std::shared_ptr<EditorTool>> tools = {
        { EditorTool::Type::TransformTool,   std::make_shared<TransformTool>() },
        { EditorTool::Type::FoliagePainter,   std::make_shared<FoliagePaintTool>() },
        { EditorTool::Type::TerrainDeformer,   std::make_shared<TerrainDeformTool>() },
        //{ EditorTool::Type::TerrainSculpt,  std::make_shared<TerrainSculptTool>() },
        //{ EditorTool::Type::TerrainPaint,   std::make_shared<TerrainPaintTool>() },
        //{ EditorTool::Type::Select,         std::make_shared<SelectTool>() },
        //{ EditorTool::Type::Move,           std::make_shared<MoveTool>() }
    };

    auto it = tools.find(type);
    if (it != tools.end())
        return it->second;

    return nullptr;
}