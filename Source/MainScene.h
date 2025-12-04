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

#pragma once

#include "axmol/axmol.h"

class MainScene : public ax::Scene
{
    enum class GameState
    {
        init = 0,
        update,
        pause,
        end,
        menu1,
        menu2,
    };

public:
    static ax::Scene* createScene();
    bool init() override;
    void onEnter() override;  // Initialize physics when scene becomes active
    void initPhysicsObjects();  // Initialize physics bodies after scene is set up
    void update(float delta) override;

    // touch
    void onTouchesBegan(const std::vector<ax::Touch*>& touches, ax::Event* event);
    void onTouchesMoved(const std::vector<ax::Touch*>& touches, ax::Event* event);
    void onTouchesEnded(const std::vector<ax::Touch*>& touches, ax::Event* event);

    // mouse
    bool onMouseDown(ax::Event* event);
    bool onMouseUp(ax::Event* event);
    bool onMouseMove(ax::Event* event);
    bool onMouseScroll(ax::Event* event);

    // Keyboard
    void onKeyPressed(ax::EventKeyboard::KeyCode code, ax::Event* event);
    void onKeyReleased(ax::EventKeyboard::KeyCode code, ax::Event* event);

    // a selector callback
    void menuCloseCallback(ax::Object* sender);

    MainScene();
    ~MainScene() override;

private:
    GameState _gameState                            = GameState::init;
    ax::EventListenerTouchAllAtOnce* _touchListener = nullptr;
    ax::EventListenerKeyboard* _keyboardListener    = nullptr;
    ax::EventListenerMouse* _mouseListener          = nullptr;
    int _sceneID                                    = 0;
    ax::Scene* _physicsScene                        = nullptr;
    ax::PhysicsWorld* _physicsWorld                 = nullptr;

	ax::Size _visibleSize;
    ax::Sprite* _sprBomb;
    ax::Sprite* _sprPlayer;
    ax::Vector<ax::Sprite*> _bombs;
    ax::MenuItemImage* _muteItem;
    ax::MenuItemImage* _unmuteItem;
    int _score;
    int _musicId;
    void initPhysics();
    void pauseCallback(ax::Object* pSender);
    void muteCallback(ax::Object* pSender);
    bool onCollision(ax::PhysicsContact& contact);
    void setPhysicsBody(ax::Sprite* sprite);
    void initTouch();
    void movePlayerByTouch(ax::Touch* touch, ax::Event* event);
    bool explodeBombs(ax::Touch* touch, ax::Event* event);
    void movePlayerIfPossible(float newX);
    void movePlayerByAccelerometer(ax::Acceleration* acceleration, ax::Event* event);
    void initAccelerometer();
    void initBackButtonListener();
    void updateScore(float dt);
    void addBombs(float dt);
    void initAudio();
    void initAudioNewEngine();
    void initMuteButton();
};
