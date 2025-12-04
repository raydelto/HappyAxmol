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

#include "GameOverScene.h"
#include "MainScene.h"

using namespace ax;

ax::Scene* GameOver::createScene()
{
    return utils::createInstance<GameOver>();
}

bool GameOver::init()
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
        AX_CALLBACK_1(GameOver::exitPause, this));

    pauseItem->setPosition(Vec2(origin.x + visibleSize.width - pauseItem->getContentSize().width / 2,
        origin.y + pauseItem->getContentSize().height / 2));

    auto menu = Menu::create(pauseItem, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    auto bg = Sprite::create("background.png");
    bg->setAnchorPoint(Vec2());
    bg->setPosition(0, 0);
    this->addChild(bg, -1);

    auto label = Label::createWithTTF("Game Over", "fonts/Marker Felt.ttf", 96);
    label->enableOutline(Color32(255, 0, 0, 100), 6);
    label->enableGlow(Color32(255, 0, 0, 255));
    label->enableShadow();
    label->setPosition(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2);
    this->addChild(label, 1);

    auto label2 = Label::createWithSystemFont("Your score is", "Arial", 48);
    label2->setPosition(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2.5);
    this->addChild(label2, 1);

    char scoreText[32];
    int score = UserDefault::getInstance()->getIntegerForKey("score", 0);
    sprintf(scoreText, "%d", score);
    auto label3 = Label::createWithBMFont("font.fnt", scoreText);
    label3->setPosition(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 3.5);
    this->addChild(label3, 1);

    return true;
}

void GameOver::exitPause(ax::Object* pSender)
{
    Director::getInstance()->replaceScene(TransitionFlipX::create(1.0, MainScene::createScene()));
}
