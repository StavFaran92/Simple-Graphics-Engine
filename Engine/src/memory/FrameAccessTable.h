#pragma once

#include <deque>
#include <unordered_set>
#include <cstdint>

#include "memory/ResourceRef.h"
#include "systems/SubSystem.h"


class FrameAccessTable : public SubSystem
{
public:
    explicit FrameAccessTable(int windowSize);

    void onResourceAccessed(ResourceRef<Resource> ref);
    void nextFrame();

    uint64_t getCurrentFrame() const;
    int getWindowSize() const;

private:
    struct FrameEntry
    {
        struct ResourceRefHash
        {
            size_t operator()(const ResourceRef<Resource>& ref) const
            {
                return std::hash<ResourceID>{}(ref.getUID());
            }
        };

        struct ResourceRefEqual
        {
            bool operator()(const ResourceRef<Resource>& a, const ResourceRef<Resource>& b) const
            {
                return a.getUID() == b.getUID();
            }
        };

        uint64_t frameNumber = 0;
        std::unordered_set<ResourceRef<Resource>, ResourceRefHash, ResourceRefEqual> resources;
    };

    std::deque<FrameEntry> m_frames;
    uint64_t m_currentFrame = 0;
    int m_windowSize;
};
