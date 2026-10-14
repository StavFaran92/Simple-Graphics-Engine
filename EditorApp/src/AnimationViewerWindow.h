#pragma once

#include "runtime/Entity.h"
#include "memory/AssetAliases.h"
#include "PreviewRenderer.h"

class AnimationViewerWindow
{
public:
    static void open(Entity entity, AnimationAssetRef animation);
    static void display();

};
