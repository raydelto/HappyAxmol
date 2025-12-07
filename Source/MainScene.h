#pragma once

#include "axmol/axmol.h"

class MainScene : public ax::Node
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
    void onEnter() override;
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
    GameState _gameState;
    ax::EventListenerTouchAllAtOnce* _touchListener;
    ax::EventListenerKeyboard* _keyboardListener;
    ax::EventListenerMouse* _mouseListener;
    ax::Director* _director;

	ax::Size _visibleSize;
    ax::Sprite* _sprPlayer;
    ax::Vector<ax::Sprite*> _bombs;
    ax::MenuItemImage* _muteItem;
    ax::MenuItemImage* _unmuteItem;
    int _score;
    int _musicId;
    void pauseCallback(ax::Object* pSender);
    void muteCallback(ax::Object* pSender);
    void onCollision();
    void initTouch();
    void movePlayerByTouch(ax::Touch* touch, ax::Event* event);
    bool explodeBombs(ax::Touch* touch, ax::Event* event);
    void movePlayerIfPossible(float newX);
    void movePlayerByAccelerometer(ax::Acceleration* acceleration, ax::Event* event);
    void initAccelerometer();
    void initBackButtonListener();
    void updateScore(float dt);
    void addBombs(float dt);
    void initAudioNewEngine();
    void initMuteButton();
};
