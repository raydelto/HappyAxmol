#include "MainScene.h"
#include "GameOverScene.h"
#include "PauseScene.h"
#include "axmol/audio/AudioEngine.h"

using namespace ax;

static int s_sceneID = 1000;

ax::Scene* MainScene::createScene()
{
    auto scene = Scene::create();
    auto layer = utils::createInstance<MainScene>();
    scene->addChild(layer);

    return scene;
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf(
        "Depending on how you compiled you might have to add 'Content/' in front of filenames in "
        "MainScene.cpp\n");
}

// on "init" you need to initialize your instance
bool MainScene::init()
{
    //////////////////////////////
    // 1. super init first
    if (!Node::init())
    {
        return false;
    }

    _score       = 0;
    _director    = Director::getInstance();
    _visibleSize = _director->getVisibleSize();

    auto pauseItem =
        ax::MenuItemImage::create("pause.png", "pause_pressed.png", AX_CALLBACK_1(MainScene::pauseCallback, this));
    if (!pauseItem)
    {
        problemLoading("pause.png");
        return false;
    }

    pauseItem->setPosition(
        Vec2(_visibleSize.width - pauseItem->getContentSize().width / 2, pauseItem->getContentSize().height / 2));

    auto menu = Menu::create(pauseItem, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    auto bg = Sprite::create("background.png");
    if (!bg)
    {
        problemLoading("background.png");
        return false;
    }
    bg->setAnchorPoint(Vec2());
    bg->setPosition(0, 0);
    this->addChild(bg, -1);

    _sprPlayer = Sprite::create("player.png");
    if (!_sprPlayer)
    {
        problemLoading("player.png");
        return false;
    }
    _sprPlayer->setPosition(_visibleSize.width / 2, _visibleSize.height * 0.23);
    this->addChild(_sprPlayer, 0);

    // Animations
    Vector<SpriteFrame*> frames;
    Size playerSize = _sprPlayer->getContentSize();
    frames.pushBack(SpriteFrame::create("player.png", Rect(0, 0, playerSize.width, playerSize.height)));
    frames.pushBack(SpriteFrame::create("player2.png", Rect(0, 0, playerSize.width, playerSize.height)));
    auto animation = Animation::createWithSpriteFrames(frames, 0.2f);
    auto animate   = Animate::create(animation);
    _sprPlayer->runAction(RepeatForever::create(animate));

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

void MainScene::movePlayerByTouch(Touch* touch, Event* event)
{
    Vec2 touchLocation = touch->getLocation();
    if (_sprPlayer->getBoundingBox().containsPoint(touchLocation))
    {
        movePlayerIfPossible(touchLocation.x);
    }
}

bool MainScene::explodeBombs(ax::Touch* touch, ax::Event* event)
{
    Vec2 touchLocation = touch->getLocation();
    ax::Vector<ax::Sprite*> toErase;

    for (auto bomb : _bombs)
    {
        if (bomb->getBoundingBox().containsPoint(touchLocation))
        {
            ax::AudioEngine::play2d("bomb.mp3");
            auto explosion = ParticleExplosion::create();
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
    auto listener          = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = AX_CALLBACK_2(MainScene::explodeBombs, this);
    listener->onTouchMoved = AX_CALLBACK_2(MainScene::movePlayerByTouch, this);
    listener->onTouchEnded = [=](Touch* touch, Event* event) {};
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

void MainScene::initAccelerometer()
{
    Device::setAccelerometerEnabled(true);
    auto listener = EventListenerAcceleration::create(AX_CALLBACK_2(MainScene::movePlayerByAccelerometer, this));
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
        AudioEngine::stopAll();
        AudioEngine::play2d("uh.mp3");
    }

    UserDefault::getInstance()->setIntegerForKey("score", _score);
    _director->replaceScene(TransitionFlipX::create(1.0, GameOver::createScene()));
}

void MainScene::updateScore(float dt)
{
    _score += 10;
}

void MainScene::addBombs(float dt)
{
    for (int i = 0; i < 3; i++)
    {
        auto bomb    = Sprite::create("bomb.png");
        float* speed = new float;
        *speed       = ax::random(1.0f, 5.5f);
        bomb->setUserData(static_cast<void*>(speed));
        bomb->setPosition(AXRANDOM_0_1() * _visibleSize.width, _visibleSize.height + bomb->getContentSize().height / 2);
        this->addChild(bomb, 1);
        _bombs.pushBack(bomb);
    }
}

void MainScene::initAudioNewEngine()
{
    if (AudioEngine::lazyInit())
    {
        _musicId = AudioEngine::play2d("music.mp3");
        AudioEngine::setVolume(_musicId, 1);
        AudioEngine::setLoop(_musicId, true);
        AXLOGD("Audio initialized successfully");
    }
    else
    {
        AXLOGD("Error while initializing new audio engine");
    }
    AudioEngine::preload("uh.mp3");
}

void MainScene::initMuteButton()
{
    _muteItem = MenuItemImage::create("mute.png", "mute.png", AX_CALLBACK_1(MainScene::muteCallback, this));

    _muteItem->setPosition(Vec2(_visibleSize.width - _muteItem->getContentSize().width / 2,
                                _visibleSize.height - _muteItem->getContentSize().height * 2));

    _unmuteItem = MenuItemImage::create("unmute.png", "unmute.png", AX_CALLBACK_1(MainScene::muteCallback, this));

    _unmuteItem->setPosition(Vec2(_visibleSize.width - _unmuteItem->getContentSize().width / 2,
                                  _visibleSize.height - _unmuteItem->getContentSize().height * 2));
    _unmuteItem->setVisible(false);

    auto menu = Menu::create(_muteItem, _unmuteItem, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 2);
}

void MainScene::muteCallback(ax::Object* pSender)
{

    AudioEngine::setVolume(_musicId, _unmuteItem->isVisible());

    _muteItem->setVisible(!_muteItem->isVisible());
    _unmuteItem->setVisible(!_muteItem->isVisible());
}

void MainScene::pauseCallback(ax::Object* pSender)
{
    _director->pushScene(TransitionFlipX::create(0.0, Pause::createScene()));
}

void MainScene::initBackButtonListener()
{
    auto listener           = EventListenerKeyboard::create();
    listener->onKeyPressed  = [=](EventKeyboard::KeyCode keyCode, Event* event) {};
    listener->onKeyReleased = AX_CALLBACK_2(MainScene::onKeyPressed, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

void MainScene::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event)
{
    if (keyCode == EventKeyboard::KeyCode::KEY_BACK)
    {
        Director::getInstance()->end();
    }
}

void MainScene::menuCloseCallback(ax::Object* sender)
{
    Director::getInstance()->end();
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
        constexpr float BOMB_SPEED = 5.0f;
        ax::Vector<ax::Sprite*> toErase;

        for (auto bomb : _bombs)
        {
            bomb->setPositionY(bomb->getPositionY() - (BOMB_SPEED + static_cast<float*>(bomb->getUserData())[0]));
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
{
    _sceneID = ++s_sceneID;
}

MainScene::~MainScene()
{

    if (_touchListener)
        _eventDispatcher->removeEventListener(_touchListener);
    if (_keyboardListener)
        _eventDispatcher->removeEventListener(_keyboardListener);
    if (_mouseListener)
        _eventDispatcher->removeEventListener(_mouseListener);
    _sceneID = -1;
}
