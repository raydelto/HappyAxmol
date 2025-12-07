#include "PauseScene.h"

Pause::Pause() : _director(nullptr), _visibleSize(ax::Size()) {}

ax::Scene* Pause::createScene()
{
    return ax::utils::createInstance<Pause>();
}

bool Pause::init()
{
    if (!Scene::init())
    {
        return false;
    }

    _director       = ax::Director::getInstance();
    _visibleSize    = _director->getVisibleSize();
    ax::Vec2 origin = _director->getVisibleOrigin();

    auto pauseItem = ax::MenuItemImage::create("play.png", "play_pressed.png", AX_CALLBACK_1(Pause::exitPause, this));

    pauseItem->setPosition(ax::Vec2(origin.x + _visibleSize.width - pauseItem->getContentSize().width / 2,
                                    origin.y + pauseItem->getContentSize().height / 2));

    auto menu = ax::Menu::create(pauseItem, nullptr);
    menu->setPosition(ax::Vec2::ZERO);
    this->addChild(menu, 1);

    auto lblPause = ax::Label::createWithTTF("PAUSE", "fonts/Marker Felt.ttf", 96);
    lblPause->setPosition(origin.x + _visibleSize.width / 2, origin.y + _visibleSize.height / 2);
    this->addChild(lblPause, 1);

    return true;
}

void Pause::exitPause(ax::Object* pSender)
{
    _director->popScene();
}
