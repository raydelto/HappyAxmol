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

#include "MainScene.h"
#include "GameOverScene.h"
#include "PauseScene.h"
#include "axmol/audio/AudioEngine.h"

using namespace ax;

static int s_sceneID = 1000;

ax::Scene* MainScene::createScene()
{
    auto scene = Scene::createWithPhysics();
    auto physicsWorld = scene->getPhysicsWorld();
    physicsWorld->setGravity(ax::Vec2(0, 0));
    
    auto layer = utils::createInstance<MainScene>();
    layer->_physicsScene = scene;  // Store reference to physics scene
    layer->_physicsWorld = physicsWorld;  // Store reference to physics world
    scene->addChild(layer);
    
    // Physics objects will be initialized in onEnter() when scene becomes running
    
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
    if (!Scene::init())
    {
        return false;
    }

    _score       = 0;
    _director    = Director::getInstance();
    _visibleSize = _director->getVisibleSize();
    auto origin  = _director->getVisibleOrigin();
    auto safeArea    = _director->getSafeAreaRect();
    auto safeOrigin  = safeArea.origin;

    auto closeItem = ax::MenuItemImage::create("pause.png", "pause_pressed.png",
                                               AX_CALLBACK_1(MainScene::pauseCallback, this));
    if (!closeItem)
    {
        problemLoading("pause.png");
        return false;
    }

    closeItem->setPosition(
        Vec2(_visibleSize.width - closeItem->getContentSize().width / 2, closeItem->getContentSize().height / 2));

    auto menu = Menu::create(closeItem, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);
    
    _sprBomb = Sprite::create("bomb.png");
    if (!_sprBomb)
    {
        problemLoading("bomb.png");
        return false;
    }
    _sprBomb->setPosition(_visibleSize.width / 2, _visibleSize.height + _sprBomb->getContentSize().height / 2);
    this->addChild(_sprBomb, 1);
    
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

    // Physics bodies will be set up in initPhysicsObjects() after scene hierarchy is complete
    initTouch();
    initAccelerometer();
    initBackButtonListener();
    schedule(AX_SCHEDULE_SELECTOR(MainScene::updateScore), 3.0f);
    schedule(AX_SCHEDULE_SELECTOR(MainScene::addBombs), 8.0f);
    initAudioNewEngine();
    initMuteButton();
    _bombs.pushBack(_sprBomb);

    // scheduleUpdate() is required to ensure update(float) is called on every loop
    scheduleUpdate();

    return true;
}

void MainScene::onEnter()
{
    Scene::onEnter();
    
    // Now the scene is running, we can initialize physics objects
    initPhysicsObjects();
}

void MainScene::initPhysicsObjects()
{
    // Now we're part of the physics scene, so we can safely create physics bodies
    setPhysicsBody(_sprPlayer);
    setPhysicsBody(_sprBomb);
    
    initPhysics();
    
    // Verify bomb physics body was created successfully
    auto bombBody = _sprBomb->getPhysicsBody();
    if (bombBody)
    {
        bombBody->setVelocity(ax::Vec2(0, -100));
    }
    else
    {
        AXLOGD("Warning: Failed to create physics body for initial bomb");
    }
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
            // auto explosion = ParticleSystemQuad::create("explosion.plist");
            explosion->setPosition(bomb->getPosition());
            this->addChild(explosion);
            /*explosion->setTotalParticles(800);
            explosion->setSpeed(3.5f);
            explosion->setLife(300.0f);	*/
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

bool MainScene::onCollision(PhysicsContact& contact)
{
    AXLOGD("Collision detected");
    auto playerBody = _sprPlayer->getPhysicsBody();
    auto shapeA = contact.getShapeA();
    auto shapeB = contact.getShapeB();
    
    // Check if player is involved in the collision
    if (shapeA->getBody() != playerBody && shapeB->getBody() != playerBody)
    {
        return false;
    }

    if (_muteItem->isVisible())
    {
        // CocosDenshion
        /*SimpleAudioEngine::getInstance()->stopBackgroundMusic();
        SimpleAudioEngine::getInstance()->playEffect("uh.wav");*/

        // New audio engine
        AudioEngine::stopAll();
        AudioEngine::play2d("uh.mp3");
    }

    UserDefault::getInstance()->setIntegerForKey("score", _score);
    _director->replaceScene(TransitionFlipX::create(1.0, GameOver::createScene()));
    auto body = _sprBomb->getPhysicsBody();
    body->setVelocity(ax::Vec2());
    body->applyTorque(100900.5f);
    return false;
}

void MainScene::setPhysicsBody(ax::Sprite* sprite)
{
    // Verify sprite exists
    if (!sprite)
    {
        AXLOGD("Cannot create physics body: sprite is null");
        return;
    }
    
    // Debug: Check what we have access to
    auto director = Director::getInstance();
    auto runningScene = director ? director->getRunningScene() : nullptr;
    auto physicsWorld = runningScene ? runningScene->getPhysicsWorld() : nullptr;
    
    AXLOGD("setPhysicsBody: director={}, runningScene={}, physicsWorld={}, _physicsScene={}",
           (void*)director, (void*)runningScene, (void*)physicsWorld, (void*)_physicsScene);
    
    // Verify we have access to physics world through running scene
    if (!physicsWorld)
    {
        AXLOGD("Cannot create physics body: no physics world accessible from running scene");
        return;
    }
    
    auto body = PhysicsBody::createCircle(sprite->getContentSize().width / 2);
    if (!body)
    {
        AXLOGD("PhysicsBody::createCircle returned null");
        return;
    }
    
    body->setContactTestBitmask(true);
    body->setDynamic(true);
    sprite->setPhysicsBody(body);
}

void MainScene::updateScore(float dt)
{
    _score += 10;
}

void MainScene::addBombs(float dt)
{
    Sprite* bomb = nullptr;
    for (int i = 0; i < 3; i++)
    {
        bomb = Sprite::create("bomb.png");
        if (!bomb)
        {
            continue;  // Skip if sprite creation failed
        }
        bomb->setPosition(AXRANDOM_0_1() * _visibleSize.width, _visibleSize.height + bomb->getContentSize().height / 2);
        
        // Add to this MainScene (which is child of physics scene)
        this->addChild(bomb, 1);
        
        setPhysicsBody(bomb);      // Then create physics body
        
        // Verify physics body was created before using it
        auto physicsBody = bomb->getPhysicsBody();
        if (physicsBody)
        {
            physicsBody->setVelocity(ax::Vec2(0, ((AXRANDOM_0_1() + 0.2f) * -250)));
            _bombs.pushBack(bomb);
        }
        else
        {
            AXLOGD("Failed to create physics body for bomb");
            this->removeChild(bomb);  // Clean up if physics body creation failed
        }
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
}

void MainScene::initAudio()
{
    int musicHandler = ax::AudioEngine::play2d("music.mp3", true);
    ax::AudioEngine::preload("uh.wav");

    ax::AudioEngine::setVolume(musicHandler, 1.0f);
}

void MainScene::initPhysics()
{
    auto contactListener            = EventListenerPhysicsContact::create();
    contactListener->onContactBegin = AX_CALLBACK_1(MainScene::onCollision, this);
    getEventDispatcher()->addEventListenerWithSceneGraphPriority(contactListener, this);
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
    if (_muteItem->isVisible())
    {
        // CocosDenshion
        // SimpleAudioEngine::getInstance()->setBackgroundMusicVolume(0);
        AudioEngine::setVolume(_musicId, 0);
    }
    else
    {
        // SimpleAudioEngine::getInstance()->setBackgroundMusicVolume(1);
        AudioEngine::setVolume(_musicId, 1);
    }

    _muteItem->setVisible(!_muteItem->isVisible());
    _unmuteItem->setVisible(!_muteItem->isVisible());
}



void MainScene::pauseCallback(ax::Object* pSender)
{
    _director->pushScene(TransitionFlipX::create(1.0, Pause::createScene()));
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
    // Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();
}

void MainScene::update(float delta)
{
    switch (_gameState)
    {
    case GameState::init:
    {
        _gameState = GameState::update;
        break;
    }

    case GameState::update:
    {
        /////////////////////////////
        // Add your codes below...like....
        //
        // UpdateJoyStick();
        // UpdatePlayer();
        // UpdatePhysics();
        // ...
        break;
    }

    case GameState::pause:
    {
        /////////////////////////////
        // Add your codes below...like....
        //
        // anyPauseStuff()

        break;
    }

    case GameState::menu1:
    {  /////////////////////////////
        // Add your codes below...like....
        //
        // UpdateMenu1();
        break;
    }

    case GameState::menu2:
    {  /////////////////////////////
        // Add your codes below...like....
        //
        // UpdateMenu2();
        break;
    }

    case GameState::end:
    {  /////////////////////////////
        // Add your codes below...like....
        //
        // CleanUpMyCrap();
        menuCloseCallback(this);
        break;
    }

    }  // switch
}



MainScene::MainScene()
{
    _sceneID = ++s_sceneID;
    AXLOGD("Scene: ctor: #{}", _sceneID);
}

MainScene::~MainScene()
{
    AXLOGD("~Scene: dtor: #{}", _sceneID);

    if (_touchListener)
        _eventDispatcher->removeEventListener(_touchListener);
    if (_keyboardListener)
        _eventDispatcher->removeEventListener(_keyboardListener);
    if (_mouseListener)
        _eventDispatcher->removeEventListener(_mouseListener);
    _sceneID = -1;
}
