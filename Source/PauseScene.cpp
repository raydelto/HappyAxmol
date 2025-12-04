/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 Copyright (c) 2019-present Axmol Engine contributors (see AUTHORS.md).

 https://axmol.dev/

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "PauseScene.h"

using namespace ax;

ax::Scene* Pause::createScene()
{
    return utils::createInstance<Pause>();
}

bool Pause::init()
{
    if (!Scene::init())
    {
        return false;
    }

    director = Director::getInstance();
    visibleSize = director->getVisibleSize();
    Vec2 origin = director->getVisibleOrigin();

    auto pauseItem = MenuItemImage::create(
        "play.png",
        "play_pressed.png",
        AX_CALLBACK_1(Pause::exitPause, this));

    pauseItem->setPosition(Vec2(origin.x + visibleSize.width - pauseItem->getContentSize().width / 2,
        origin.y + pauseItem->getContentSize().height / 2));

    auto menu = Menu::create(pauseItem, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    auto label = Label::createWithTTF("PAUSE", "fonts/Marker Felt.ttf", 96);
    label->setPosition(origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height / 2);
    this->addChild(label, 1);

    return true;
}

void Pause::exitPause(ax::Object* pSender)
{
    // Pop the pause scene from the Scene stack.
    // This will remove current scene.
    Director::getInstance()->popScene();
}
