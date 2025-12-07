#pragma once

#include "axmol/axmol.h"

class GameOver : public ax::Scene
{
public:
    GameOver();
    ~GameOver() = default;
    static ax::Scene* createScene();
    bool init() override;
    void exit(ax::Object* pSender);

private:
    ax::Director* _director;
    ax::Size _visibleSize;
};
