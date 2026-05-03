#include "memory/FrameAccessTable.h"

#include "core/Engine.h"

FrameAccessTable::FrameAccessTable(int windowSize)
    : m_windowSize(windowSize)
{
    Engine::get()->registerSubSystem<FrameAccessTable>(this);

    m_frames.push_back({ m_currentFrame, {} });
}

void FrameAccessTable::onResourceAccessed(ResourceRef<Resource> ref)
{
    if (ref.isEmpty())
        return;

    m_frames.back().resources.insert(ref);
}

void FrameAccessTable::nextFrame()
{
    m_frames.push_back({ ++m_currentFrame, {} });

    if ((int)m_frames.size() > m_windowSize)
        m_frames.pop_front();
}

uint64_t FrameAccessTable::getCurrentFrame() const
{
    return m_currentFrame;
}

int FrameAccessTable::getWindowSize() const
{
    return m_windowSize;
}
