#pragma once

#include "runtime/Entity.h"
#include "memory/AssetAliases.h"
#include "AnimationViewRenderer.h"

class AnimationViewerWindow
{
public:
    static void open(Entity entity, AnimationAssetRef animation);
    static void display();

};
