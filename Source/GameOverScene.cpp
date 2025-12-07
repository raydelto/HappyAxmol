#include "GameOverScene.h"
#include "MainScene.h"

ax::Scene* GameOver::createScene()
{
    return ax::utils::createInstance<GameOver>();
}

GameOver::GameOver() : _director(nullptr), _visibleSize(ax::Size()) {}

bool GameOver::init()
{
    if (!Scene::init())
    {
        return false;
    }

    _director       = ax::Director::getInstance();
    _visibleSize    = _director->getVisibleSize();
    ax::Vec2 origin = _director->getVisibleOrigin();

    auto playItem = ax::MenuItemImage::create("play.png", "play_pressed.png", AX_CALLBACK_1(GameOver::exit, this));

    playItem->setPosition(ax::Vec2(origin.x + _visibleSize.width - playItem->getContentSize().width / 2,
                                   origin.y + playItem->getContentSize().height / 2));

    auto menu = ax::Menu::create(playItem, nullptr);
    menu->setPosition(ax::Vec2::ZERO);
    this->addChild(menu, 1);

    auto bg = ax::Sprite::create("background.png");
    bg->setAnchorPoint(ax::Vec2());
    bg->setPosition(0, 0);
    this->addChild(bg, -1);

    auto lblGameOver = ax::Label::createWithTTF("Game Over", "fonts/Marker Felt.ttf", 96);
    lblGameOver->enableOutline(ax::Color32(255, 0, 0, 100), 6);
    lblGameOver->enableGlow(ax::Color32(255, 0, 0, 255));
    lblGameOver->enableShadow();
    lblGameOver->setPosition(origin.x + _visibleSize.width / 2, origin.y + _visibleSize.height / 2);
    this->addChild(lblGameOver, 1);

    auto lblScoreText = ax::Label::createWithSystemFont("Your score is", "Arial", 48);
    lblScoreText->setPosition(origin.x + _visibleSize.width / 2, origin.y + _visibleSize.height / 2.5);
    this->addChild(lblScoreText, 1);

    char scoreText[32];
    int score = ax::UserDefault::getInstance()->getIntegerForKey("score", 0);
    sprintf(scoreText, "%d", score);
    auto lblScoreNumber = ax::Label::createWithBMFont("font.fnt", scoreText);
    lblScoreNumber->setPosition(origin.x + _visibleSize.width / 2, origin.y + _visibleSize.height / 3.5);
    this->addChild(lblScoreNumber, 1);

    return true;
}

void GameOver::exit(ax::Object* pSender)
{
    _director->replaceScene(ax::TransitionFlipX::create(0.0, MainScene::createScene()));
}
