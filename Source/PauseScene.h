#pragma once

#include "axmol/axmol.h"

class Pause : public ax::Scene
{
public:
    Pause();
    ~Pause() = default;
    static ax::Scene* createScene();
    bool init() override;
    void exitPause(ax::Object* pSender);

private:
    ax::Director* _director;
    ax::Size _visibleSize;
};
