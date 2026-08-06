#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "audio.h"
#include "constants.h"
#include "fx.h"
#include "game.h"
#include "input.h"
#include "renderer.h"
#include "storage.h"
#include "ui.h"

using namespace neon;

// --aim-test 无头自检:验证粘球发射方向计算(方向向量/最小仰角/默认回退)
static int runAimTest() {
  int failures = 0;
  auto check = [&](const char* name, float dx, float dy, float ex, float ey, float tol) {
    bool ok = std::abs(dx - ex) < tol && std::abs(dy - ey) < tol;
    std::printf("[%s] %s: (%.3f, %.3f) expect (%.3f, %.3f)\n", ok ? "PASS" : "FAIL", name, dx, dy, ex, ey);
    if (!ok) failures++;
  };
  float dx, dy;
  // 用例 1:球(480,545)→ 鼠标(800,100),向量(320,-445) 归一化 ≈ (0.584, -0.812)
  Game::computeLaunchDirection(dx, dy, 480, 545, 800, 100);
  check("右上方向", dx, dy, 0.584f, -0.812f, 0.01f);
  // 用例 2:鼠标在球正下方 → 强制最小仰角(垂直向上)
  Game::computeLaunchDirection(dx, dy, 480, 545, 480, 600);
  check("正下方抬升", dx, dy, 0.0f, -1.0f, 0.01f);
  // 用例 3:鼠标与球重合 → 回退默认方向(0.55, -0.835) 归一化
  Game::computeLaunchDirection(dx, dy, 480, 545, 480, 545);
  check("重合回退默认", dx, dy, 0.550f, -0.835f, 0.01f);
  // 用例 4:鼠标左下方 → 负 x + 最小仰角(-0.989, -0.150)
  Game::computeLaunchDirection(dx, dy, 480, 545, 100, 600);
  check("左下抬升", dx, dy, -0.989f, -0.150f, 0.01f);
  // 用例 5:接近水平(鼠标与球同高偏右)→ 仍带最小仰角
  Game::computeLaunchDirection(dx, dy, 480, 545, 700, 545);
  check("水平抬升", dx, dy, 0.989f, -0.150f, 0.01f);
  if (failures == 0) { std::printf("aim-test: ALL PASS\n"); return 0; }
  std::printf("aim-test: %d FAILURE(S)\n", failures);
  return 1;
}

// 与 web 版 main.ts 对应的入口:装配 + 事件循环 + 每帧渲染
int main(int argc, char* argv[]) {
  std::string screenshotPath;
  int startLevel = 1;
  bool autoStart = false;
  bool aimTest = false;
  int resizeW = 0, resizeH = 0;
  for (int i = 1; i < argc; i++) {
    if (std::strcmp(argv[i], "--screenshot") == 0 && i + 1 < argc) screenshotPath = argv[++i];
    else if (std::strcmp(argv[i], "--level") == 0 && i + 1 < argc) startLevel = std::atoi(argv[++i]);
    else if (std::strcmp(argv[i], "--auto") == 0) autoStart = true;
    else if (std::strcmp(argv[i], "--aim-test") == 0) aimTest = true;
    else if (std::strcmp(argv[i], "--resize") == 0 && i + 1 < argc) {
      if (std::sscanf(argv[++i], "%dx%d", &resizeW, &resizeH) != 2) { resizeW = resizeH = 0; }
    }
  }
  if (aimTest) return runAimTest();
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
    std::fprintf(stderr, "SDL_Init 失败: %s\n", SDL_GetError());
    return 1;
  }
  if (!TTF_Init()) {
    std::fprintf(stderr, "TTF_Init 失败: %s\n", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  // 窗口标志:桌面可缩放;移动端全屏(方向锁在工程配置层)
#if defined(__ANDROID__) || defined(__IPHONEOS__)
  const Uint32 winFlags = SDL_WINDOW_FULLSCREEN;
#else
  const Uint32 winFlags = SDL_WINDOW_RESIZABLE;
#endif
  SDL_Window* window = SDL_CreateWindow("Neon Breaker \xc2\xb7 \xe9\x9c\x93\xe8\x99\xb9\xe7\xa0\xb4\xe7\x95\x8c",
                                        Renderer::WINDOW_W, Renderer::WINDOW_H, winFlags);
  if (!window) {
    std::fprintf(stderr, "SDL_CreateWindow 失败: %s\n", SDL_GetError());
    TTF_Quit();
    SDL_Quit();
    return 1;
  }
#if !defined(__ANDROID__) && !defined(__IPHONEOS__)
  SDL_SetWindowMinimumSize(window, 640, 420); // 等比缩放下限,保证布局可用
#endif
  SDL_Renderer* sdlRenderer = SDL_CreateRenderer(window, nullptr);
  if (!sdlRenderer) {
    std::fprintf(stderr, "SDL_CreateRenderer 失败: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 1;
  }

  Renderer renderer;
  if (!renderer.init(window, sdlRenderer)) {
    std::fprintf(stderr, "警告: 未找到中文字体,文本将无法渲染\n");
  }

  AudioManager audio(true);
  audio.unlock(); // 桌面端直接打开音频设备(浏览器版需要用户手势)

  GameSnapshot snapshot;
  FxSystem fx;
  InputManager input({ 0, Renderer::WORLD_OFFSET_Y, WORLD_WIDTH, WORLD_HEIGHT }, [](InputMethod) {});
  input.setScreenTransform(renderer.scale(), renderer.offsetX(), renderer.offsetY(), renderer.windowWidth(), renderer.windowHeight());

  UI* uiPtr = nullptr;
  Game game(&renderer, &input, &audio, &fx, [&](const GameSnapshot& s) {
    snapshot = s;
    if (uiPtr) uiPtr->setSnapshot(s);
  });
  UI ui(&renderer, &game);
  uiPtr = &ui;

  game.applyStorage(loadStorage());
  if (startLevel > 1 && startLevel <= 5) game.selectLevel(startLevel - 1);
  if (resizeW > 0 && resizeH > 0) {
    SDL_SetWindowSize(window, resizeW, resizeH);
    renderer.onWindowResized(resizeW, resizeH); // dummy 驱动下 SetWindowSize 无效,直接传尺寸(与 RESIZED 事件幂等)
    input.setScreenTransform(renderer.scale(), renderer.offsetX(), renderer.offsetY(), renderer.windowWidth(), renderer.windowHeight());
    std::printf("window resized to %dx%d (scale=%.2f, offset=%.0f,%.0f)\n", resizeW, resizeH, renderer.scale(), renderer.offsetX(), renderer.offsetY());
  }

  bool running = true;
  uint64_t prev = SDL_GetTicksNS();
  uint64_t startTicks = prev;
  bool autoFired = false;
  while (running) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_EVENT_QUIT) {
        running = false;
      } else if (e.type == SDL_EVENT_WINDOW_RESIZED) {
        renderer.onWindowResized(e.window.data1, e.window.data2);
        input.setScreenTransform(renderer.scale(), renderer.offsetX(), renderer.offsetY(), renderer.windowWidth(), renderer.windowHeight());
      } else if (!autoStart && (e.type == SDL_EVENT_WINDOW_FOCUS_LOST || e.type == SDL_EVENT_WINDOW_MINIMIZED)) {
        game.autoPause(); // 等价于 window blur / visibilitychange 自动暂停(--auto 测试模式忽略无焦点环境)
      } else {
        if (!ui.settingsOpen()) input.handleEvent(e);
        ui.handleEvent(e);
      }
    }

    uint64_t now = SDL_GetTicksNS();
    float frame = std::min(MAX_FRAME_TIME, static_cast<float>(now - prev) / 1e9f);
    prev = now;

    // 与 web 版一致:overlay 可见时(非 playing)指针输入被遮罩挡住
    input.setPointerActive(snapshot.state == GameState::Playing && !ui.settingsOpen());

    if (autoStart && !autoFired && static_cast<float>(now - startTicks) / 1e9f > 0.3f) {
      input.queuePrimary(); // ready → playing(发射球)
      autoFired = true;
      std::printf("auto primary queued\n");
    }

    game.step(frame);

    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
    SDL_RenderClear(sdlRenderer); // 清除 letterbox 黑边区域
    WorldRenderArgs args;
    args.state = game.state();
    args.balls = &game.balls();
    args.paddle = &game.paddle();
    args.bricks = &game.bricks();
    args.respawnQueue = &game.respawnQueue();
    args.powerUps = &game.powerUps();
    args.lasers = &game.lasers();
    args.fx = &fx;
    args.shield = game.shield();
    args.effects = &game.effects();
    args.combo = game.combo();
    args.stuckBall = game.stuckBall();
    args.aimX = game.aimX();
    args.aimY = game.aimY();
    args.language = snapshot.language;
    renderer.renderWorld(args);

    ui.render();
    if (!screenshotPath.empty() && static_cast<float>(now - startTicks) / 1e9f > 0.5f) {
      SDL_Surface* shot = SDL_RenderReadPixels(sdlRenderer, nullptr);
      if (shot) {
        SDL_SaveBMP(shot, screenshotPath.c_str());
        SDL_DestroySurface(shot);
        std::printf("screenshot saved: %s (state=%d)\n", screenshotPath.c_str(), static_cast<int>(snapshot.state));
      } else {
        std::fprintf(stderr, "screenshot failed: %s\n", SDL_GetError());
      }
      break;
    }
    SDL_RenderPresent(sdlRenderer);
  }

  renderer.shutdown();
  audio.shutdown();
  SDL_DestroyRenderer(sdlRenderer);
  SDL_DestroyWindow(window);
  TTF_Quit();
  SDL_Quit();
  return 0;
}
