#pragma once

#include "nlohmann/json.hpp"

class AnimationGraph;
class Animator;

class AnimationGraphWindow
{
public:
    static void open(Animator* animator);
    static void display();
};
