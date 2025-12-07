#include "AppDelegate.h"
#include "MainScene.h"

#define USE_VR_RENDERER  0
#define USE_AUDIO_ENGINE 1

#if USE_AUDIO_ENGINE
#    include "axmol/audio/AudioEngine.h"
#endif

#if USE_VR_RENDERER && defined(AX_ENABLE_VR)
#    include "axmol/vr/VRGenericRenderer.h"
#endif


static ax::Size designResolutionSize = ax::Size(768, 1280);


// if you want a different context, modify the value of contextAttrs
// it will affect all platforms
void AppDelegate::initContextAttrs()
{
    // Set app context attributes: red,green,blue,alpha,depth,stencil,multisamplesCount
    // powerPreference only affect when RHI backend is D3D
    ax::ContextAttrs contextAttrs = {.powerPreference = ax::PowerPreference::HighPerformance};

    // V-Sync is enabled by default since axmol 2.2.
    // Uncomment to disable V-Sync and unlock FPS.
    // contextAttrs.vsync = false;

    // Enable high-DPI scaling support (non-Windows platforms only)
    // Note: cpp-tests keep the default render mode to ensure consistent performance benchmarks
#if AX_TARGET_PLATFORM != AX_PLATFORM_WIN32
    contextAttrs.renderScaleMode = ax::RenderScaleMode::Physical;
#endif
    setContextAttrs(contextAttrs);
}

bool AppDelegate::applicationDidFinishLaunching()
{
    // initialize director
    auto director   = ax::Director::getInstance();
    auto renderView = director->getRenderView();
    if (!renderView)
    {
        constexpr const char* TITLE = "Happy Bunny";
#if (AX_TARGET_PLATFORM == AX_PLATFORM_WIN32) || (AX_TARGET_PLATFORM == AX_PLATFORM_MAC) || \
    (AX_TARGET_PLATFORM == AX_PLATFORM_LINUX)
        renderView = ax::RenderViewImpl::createWithRect(
            TITLE, ax::Rect(0, 0, designResolutionSize.width, designResolutionSize.height));
#else
        renderView = RenderViewImpl::create(TITLE);
#endif
        director->setRenderView(renderView);
    }
#if USE_VR_RENDERER && defined(AX_ENABLE_VR)
    auto vrRenderer = std::make_unique<VRGenericRenderer>();
    // On Android/iOS emulator devices, uncomment to visualize the left/right eye VR rendering output.
    // Useful for debugging stereo rendering without a physical headset.
    // vrRenderer->setDebugIgnoreHeadTracker(true);
    renderView->setVR(std::move(vrRenderer));
#endif

    auto glview     = director->getRenderView();
    ax::Size screenSize = glview->getFrameSize();
    ax::Size designSize(768, 1280);
    std::vector<std::string> searchPaths;
    searchPaths.push_back("sounds");
    searchPaths.push_back("particles");

    if (screenSize.height > 800)
    {
        // High Resolution
        searchPaths.push_back("images/high");
        director->setContentScaleFactor(1280.0f / designSize.height);
    }
    else if (screenSize.height > 600)
    {
        // Mid resolution
        searchPaths.push_back("images/mid");
        director->setContentScaleFactor(800.0f / designSize.height);
    }
    else
    {
        // Low resolution
        searchPaths.push_back("images/low");
        director->setContentScaleFactor(320.0f / designSize.height);
    }
    ax::FileUtils::getInstance()->setSearchPaths(searchPaths);
    glview->setDesignResolutionSize(designSize.width, designSize.height, ax::ResolutionPolicy::SHOW_ALL);

    // turn on display FPS
    director->setStatsDisplay(true);

    // set FPS. the default value is 1.0/60 if you don't call this
    director->setAnimationInterval(1.0f / 60);

    // Set the design resolution
    renderView->setDesignResolutionSize(designResolutionSize.width, designResolutionSize.height,
                                        ax::ResolutionPolicy::SHOW_ALL);

    // create a scene. it's an autorelease object
    auto scene = MainScene::createScene();

    // run
    director->runWithScene(scene);

    return true;
}

// This function will be called when the app is inactive. Note, when receiving a phone call it is invoked.
void AppDelegate::applicationDidEnterBackground()
{
    ax::Director::getInstance()->stopAnimation();

#if USE_AUDIO_ENGINE
    ax::AudioEngine::pauseAll();
#endif
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground()
{
    ax::Director::getInstance()->startAnimation();

#if USE_AUDIO_ENGINE
    ax::AudioEngine::resumeAll();
#endif
}

void AppDelegate::applicationWillQuit() {}
