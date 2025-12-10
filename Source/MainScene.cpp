#include "MainScene.h"
#include "GameOverScene.h"
#include "PauseScene.h"
#include "axmol/audio/AudioEngine.h"

ax::Scene* MainScene::createScene()
{
    auto scene = ax::Scene::create();
    auto layer = ax::utils::createInstance<MainScene>();
    scene->addChild(layer);

    return scene;
}

static void printLoadingError(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf(
        "Depending on how you compiled you might have to add 'Content/' in front of filenames in "
        "MainScene.cpp\n");
}

bool MainScene::init()
{
    if (!Node::init())
    {
        return false;
    }

    _score       = 0;
    _director    = ax::Director::getInstance();
    _visibleSize = _director->getVisibleSize();

    auto pauseItem =
        ax::MenuItemImage::create("pause.png", "pause_pressed.png", AX_CALLBACK_1(MainScene::pauseCallback, this));
    if (!pauseItem)
    {
        printLoadingError("pause.png");
        return false;
    }

    pauseItem->setPosition(
        ax::Vec2(_visibleSize.width - pauseItem->getContentSize().width / 2, pauseItem->getContentSize().height / 2));

    auto menu = ax::Menu::create(pauseItem, nullptr);
    menu->setPosition(ax::Vec2::ZERO);
    this->addChild(menu, 1);

    auto bg = ax::Sprite::create("background.png");
    if (!bg)
    {
        printLoadingError("background.png");
        return false;
    }
    bg->setAnchorPoint(ax::Vec2());
    bg->setPosition(0, 0);
    this->addChild(bg, -1);

    _sprPlayer = ax::Sprite::create("player.png");
    if (!_sprPlayer)
    {
        printLoadingError("player.png");
        return false;
    }
    _sprPlayer->setPosition(_visibleSize.width / 2, _visibleSize.height * 0.23);
    this->addChild(_sprPlayer, 0);

    // Animations
    ax::Vector<ax::SpriteFrame*> frames;
    ax::Size playerSize = _sprPlayer->getContentSize();
    frames.pushBack(ax::SpriteFrame::create("player.png", ax::Rect(0, 0, playerSize.width, playerSize.height)));
    frames.pushBack(ax::SpriteFrame::create("player2.png", ax::Rect(0, 0, playerSize.width, playerSize.height)));
    auto animation = ax::Animation::createWithSpriteFrames(frames, 0.2f);
    auto animate   = ax::Animate::create(animation);
    _sprPlayer->runAction(ax::RepeatForever::create(animate));

    initTouch();
    initAccelerometer();
    initBackButtonListener();
    schedule(AX_SCHEDULE_SELECTOR(MainScene::updateScore), 3.0f);
    schedule(AX_SCHEDULE_SELECTOR(MainScene::addBombs), 8.0f);
    addBombs(0.0f);
    initAudioNewEngine();
    initMuteButton();
    scheduleUpdate();

    return true;
}

void MainScene::onEnter()
{
    Node::onEnter();
}

// Move the player if it does not go outside of the screen
void MainScene::movePlayerIfPossible(float newX)
{
    float sprHalfWidth = _sprPlayer->getBoundingBox().size.width / 2;
    if (newX >= sprHalfWidth && newX < _visibleSize.width - sprHalfWidth)
    {
        _sprPlayer->setPositionX(newX);
    }
}

void MainScene::movePlayerByTouch(ax::Touch* touch, ax::Event* event)
{
    ax::Vec2 touchLocation = touch->getLocation();
    if (_sprPlayer->getBoundingBox().containsPoint(touchLocation))
    {
        movePlayerIfPossible(touchLocation.x);
    }
}

bool MainScene::explodeBombs(ax::Touch* touch, ax::Event* event)
{
    ax::Vec2 touchLocation = touch->getLocation();
    ax::Vector<ax::Sprite*> toErase;

    for (auto bomb : _bombs)
    {
        if (bomb->getBoundingBox().containsPoint(touchLocation))
        {
            ax::AudioEngine::play2d("bomb.mp3");
            auto explosion = ax::ParticleExplosion::create();
            explosion->setPosition(bomb->getPosition());
            this->addChild(explosion);
            bomb->setVisible(false);
            this->removeChild(bomb);
            toErase.pushBack(bomb);
        }
    }

    for (auto bomb : toErase)
    {
        _bombs.eraseObject(bomb);
    }

    return true;
}

void MainScene::initTouch()
{
    auto listener          = ax::EventListenerTouchOneByOne::create();
    listener->onTouchBegan = AX_CALLBACK_2(MainScene::explodeBombs, this);
    listener->onTouchMoved = AX_CALLBACK_2(MainScene::movePlayerByTouch, this);
    listener->onTouchEnded = [=](ax::Touch* touch, ax::Event* event) {};
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

void MainScene::initAccelerometer()
{
    ax::Device::setAccelerometerEnabled(true);
    auto listener = ax::EventListenerAcceleration::create(AX_CALLBACK_2(MainScene::movePlayerByAccelerometer, this));
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

void MainScene::movePlayerByAccelerometer(ax::Acceleration* acceleration, ax::Event* event)
{
    movePlayerIfPossible(_sprPlayer->getPositionX() + (acceleration->x * 10));
}

void MainScene::onCollision()
{
    _gameState = GameState::pause;

    if (_muteItem->isVisible())
    {
        ax::AudioEngine::stopAll();
        ax::AudioEngine::play2d("uh.mp3");
    }

    ax::UserDefault::getInstance()->setIntegerForKey("score", _score);
    _director->replaceScene(ax::TransitionFlipX::create(1.0, GameOver::createScene()));
}

void MainScene::updateScore(float dt)
{
    _score += 10;
}

void MainScene::addBombs(float dt)
{
    for (int i = 0; i < 3; i++)
    {
        auto bomb    = ax::Sprite::create("bomb.png");
        float* speed = new float;
        *speed       = ax::random(90.0f, 180.0f);
        bomb->setUserData(static_cast<void*>(speed));
        bomb->setPosition(AXRANDOM_0_1() * _visibleSize.width, _visibleSize.height + bomb->getContentSize().height / 2);
        this->addChild(bomb, 1);
        _bombs.pushBack(bomb);
    }
}

void MainScene::initAudioNewEngine()
{
    if (ax::AudioEngine::lazyInit())
    {
        _musicId = ax::AudioEngine::play2d("music.mp3");
        ax::AudioEngine::setVolume(_musicId, 1);
        ax::AudioEngine::setLoop(_musicId, true);
        AXLOGD("Audio initialized successfully");
    }
    else
    {
        printf("Error while initializing new audio engine.\n");
    }
    ax::AudioEngine::preload("uh.mp3");
}

void MainScene::initMuteButton()
{
    _muteItem = ax::MenuItemImage::create("mute.png", "mute.png", AX_CALLBACK_1(MainScene::muteCallback, this));

    _muteItem->setPosition(ax::Vec2(_visibleSize.width - _muteItem->getContentSize().width / 2,
                                    _visibleSize.height - _muteItem->getContentSize().height * 2));

    _unmuteItem = ax::MenuItemImage::create("unmute.png", "unmute.png", AX_CALLBACK_1(MainScene::muteCallback, this));

    _unmuteItem->setPosition(ax::Vec2(_visibleSize.width - _unmuteItem->getContentSize().width / 2,
                                      _visibleSize.height - _unmuteItem->getContentSize().height * 2));
    _unmuteItem->setVisible(false);

    auto menu = ax::Menu::create(_muteItem, _unmuteItem, nullptr);
    menu->setPosition(ax::Vec2::ZERO);
    this->addChild(menu, 2);
}

void MainScene::muteCallback(ax::Object* pSender)
{

    ax::AudioEngine::setVolume(_musicId, _unmuteItem->isVisible());

    _muteItem->setVisible(!_muteItem->isVisible());
    _unmuteItem->setVisible(!_muteItem->isVisible());
}

void MainScene::pauseCallback(ax::Object* pSender)
{
    _director->pushScene(ax::TransitionFlipX::create(0.0, Pause::createScene()));
}

void MainScene::initBackButtonListener()
{
    auto listener           = ax::EventListenerKeyboard::create();
    listener->onKeyPressed  = [=](ax::EventKeyboard::KeyCode keyCode, ax::Event* event) {};
    listener->onKeyReleased = AX_CALLBACK_2(MainScene::onKeyPressed, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

void MainScene::onKeyPressed(ax::EventKeyboard::KeyCode keyCode, ax::Event* event)
{
    if (keyCode == ax::EventKeyboard::KeyCode::KEY_BACK)
    {
        _director->end();
    }
}

void MainScene::menuCloseCallback(ax::Object* sender)
{
    _director->end();
}

void MainScene::update(float delta)
{
    switch (_gameState)
    {
    case GameState::init:
        _gameState = GameState::update;
        break;

    case GameState::update:
    {
        ax::Vector<ax::Sprite*> toErase;

        for (auto bomb : _bombs)
        {
            bomb->setPositionY(bomb->getPositionY() - (static_cast<float*>(bomb->getUserData())[0]) * delta);
            if (bomb->getBoundingBox().intersectsRect(_sprPlayer->getBoundingBox()))
            {
                onCollision();
            }
            if (bomb->getPositionY() < -bomb->getContentSize().height / 2)
            {
                toErase.pushBack(bomb);
                this->removeChild(bomb);
            }
        }

        for (auto bomb : toErase)
        {
            _bombs.eraseObject(bomb);
        }

        break;
    }
    }
}

MainScene::MainScene()
    : _gameState(GameState::init)
    , _score(0)
    , _musicId(-1)
    , _sprPlayer(nullptr)
    , _director(nullptr)
    , _touchListener(nullptr)
    , _keyboardListener(nullptr)
    , _mouseListener(nullptr)
    , _muteItem(nullptr)
    , _unmuteItem(nullptr)
{
}

MainScene::~MainScene()
{
    AXLOGD("Freeing MainScene resources.");
    if (_touchListener)
        _eventDispatcher->removeEventListener(_touchListener);
    if (_keyboardListener)
        _eventDispatcher->removeEventListener(_keyboardListener);
    if (_mouseListener)
        _eventDispatcher->removeEventListener(_mouseListener);

    // Cleanup bomb user data
    for (auto bomb : _bombs)
    {
        delete static_cast<float*>(bomb->getUserData());
    }
}
