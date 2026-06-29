#include "PowderToySDL.h"
#include "SimulationConfig.h"
#include "WindowIcon.h"
#include "Config.h"
#include "gui/interface/Engine.h"
#include "graphics/Graphics.h"
#include "graphics/PixelScale.h"
#include "common/platform/Platform.h"
#include "common/clipboard/Clipboard.h"
#include "FrameSchedule.h"
#include <algorithm>
#include <iostream>

int desktopWidth = 1280;
int desktopHeight = 1024;
SDL_Window *sdl_window = nullptr;
SDL_Renderer *sdl_renderer = nullptr;
SDL_Texture *sdl_texture = nullptr;
static SDL_Texture *sdl_text_texture = nullptr;
static int textTexW = 0, textTexH = 0;
bool vsyncHint = false;
WindowFrameOps currentFrameOps;
bool momentumScroll = true;
bool showAvatars = true;
bool showLargeScreenDialog = false;
int mousex = 0;
int mousey = 0;
int mouseButton = 0;
bool mouseDown = false;
bool calculatedInitialMouse = false;
bool hasMouseMoved = false;
double correctedFrameTimeAvg = 0;
static bool prevContributesToFps = false;

static FrameSchedule tickSchedule;
static FrameSchedule drawSchedule;
static FrameSchedule clientTickSchedule;
static FrameSchedule fpsUpdateSchedule;

// Compute the window-pixel rect into which the game texture is rendered.
// Used for BOTH SDL_RenderCopy (rendering) and mouse coordinate mapping,
// so they are guaranteed to use identical arithmetic.
static SDL_Rect ComputeGameRect()
{
	int winW, winH;
	SDL_GetWindowSize(sdl_window, &winW, &winH);
	const int texW = WINDOWW * PIXEL_SCALE;
	const int texH = WINDOWH * PIXEL_SCALE;
	float scale = std::min((float)winW / texW, (float)winH / texH);
	if (currentFrameOps.Normalize().forceIntegerScaling)
		scale = std::max(1.0f, std::floor(scale));
	int w = (int)(texW * scale);
	int h = (int)(texH * scale);
	return { (winW - w) / 2, (winH - h) / 2, w, h };
}

static std::pair<int,int> WindowToGame(int wx, int wy)
{
	SDL_Rect r = ComputeGameRect();
	return {
		(wx - r.x) * WINDOWW / r.w,
		(wy - r.y) * WINDOWH / r.h,
	};
}

void StartTextInput()
{
	SDL_StartTextInput();
}

void StopTextInput()
{
	SDL_StopTextInput();
}

void SetTextInputRect(int x, int y, int w, int h)
{
	if (!sdl_window)
		return;
	SDL_Rect r = ComputeGameRect();
	SDL_Rect rect;
	rect.x = r.x + x * r.w / WINDOWW;
	rect.y = r.y + y * r.h / WINDOWH;
	rect.w = w * r.w / WINDOWW;
	rect.h = h * r.h / WINDOWH;
	SDL_SetTextInputRect(&rect);
}

void ClipboardPush(ByteString text)
{
	SDL_SetClipboardText(text.c_str());
}

ByteString ClipboardPull()
{
	return ByteString(SDL_GetClipboardText());
}

int GetModifiers()
{
	return SDL_GetModState();
}

unsigned int GetTicks()
{
	return SDL_GetTicks();
}

uint64_t GetNowNs()
{
	return uint64_t(SDL_GetTicks()) * UINT64_C(1'000'000);
}

static void CalculateMousePosition(int *x, int *y)
{
	int globalMx, globalMy;
	SDL_GetGlobalMouseState(&globalMx, &globalMy);
	int windowX, windowY;
	SDL_GetWindowPosition(sdl_window, &windowX, &windowY);
	auto [gx, gy] = WindowToGame(globalMx - windowX, globalMy - windowY);
	if (x)
		*x = gx;
	if (y)
		*y = gy;
}

void blit(pixel *vid, Graphics *g)
{
	int winW, winH;
	SDL_GetWindowSize(sdl_window, &winW, &winH);

	SDL_UpdateTexture(sdl_texture, nullptr, vid, WINDOWW * PIXEL_SCALE * sizeof(Uint32));
	SDL_RenderClear(sdl_renderer);
	SDL_Rect dst = ComputeGameRect();
	SDL_RenderCopy(sdl_renderer, sdl_texture, nullptr, &dst);

	// Native-resolution text overlay: crispy text at actual window pixel size
	if (g && g->nativeText.active && !g->nativeText.pixels.empty())
	{
		auto &nt = g->nativeText;
		if (!sdl_text_texture || textTexW != winW || textTexH != winH)
		{
			if (sdl_text_texture)
				SDL_DestroyTexture(sdl_text_texture);
			sdl_text_texture = SDL_CreateTexture(sdl_renderer,
			    SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, winW, winH);
			SDL_SetTextureBlendMode(sdl_text_texture, SDL_BLENDMODE_BLEND);
			textTexW = winW;
			textTexH = winH;
		}
		SDL_UpdateTexture(sdl_text_texture, nullptr, nt.pixels.data(), winW * sizeof(Uint32));
		SDL_RenderCopy(sdl_renderer, sdl_text_texture, nullptr, nullptr);
	}

	SDL_RenderPresent(sdl_renderer);
}

void UpdateRefreshRate()
{
	RefreshRate refreshRate;
	int displayIndex = SDL_GetWindowDisplayIndex(sdl_window);
	if (displayIndex >= 0)
	{
		SDL_DisplayMode displayMode;
		if (!SDL_GetCurrentDisplayMode(displayIndex, &displayMode) && displayMode.refresh_rate)
		{
			refreshRate = RefreshRateQueried{ displayMode.refresh_rate };
		}
	}
	ui::Engine::Ref().SetRefreshRate(refreshRate);
}

void SDLOpen()
{
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
	{
		fprintf(stderr, "Initializing SDL (video subsystem): %s\n", SDL_GetError());
		Platform::Exit(-1);
	}
	Clipboard::Init();

	SDLSetScreen();

	int displayIndex = SDL_GetWindowDisplayIndex(sdl_window);
	if (displayIndex >= 0)
	{
		SDL_Rect rect;
		if (!SDL_GetDisplayUsableBounds(displayIndex, &rect))
		{
			desktopWidth = rect.w;
			desktopHeight = rect.h;
		}
	}
	UpdateRefreshRate();

	StopTextInput();
}

void SDLClose()
{
	if (SDL_GetWindowFlags(sdl_window) & SDL_WINDOW_OPENGL)
	{
		// * nvidia-460 egl registers callbacks with x11 that end up being called
		//   after egl is unloaded unless we grab it here and release it after
		//   sdl closes the display. this is an nvidia driver weirdness but
		//   technically an sdl bug. glfw has this fixed:
		//   https://github.com/glfw/glfw/commit/9e6c0c747be838d1f3dc38c2924a47a42416c081
		SDL_GL_LoadLibrary(nullptr);
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		SDL_GL_UnloadLibrary();
	}
	SDL_Quit();
}

void SDLSetScreen()
{
	auto newFrameOps = ui::Engine::Ref().windowFrameOps;
	auto newVsyncHint = false; // TODO: DrawLimitVsync
	if (FORCE_WINDOW_FRAME_OPS == forceWindowFrameOpsEmbedded)
	{
		newFrameOps.resizable = false;
		newFrameOps.fullscreen = false;
		newFrameOps.changeResolution = false;
		newFrameOps.forceIntegerScaling = false;
	}
	if (FORCE_WINDOW_FRAME_OPS == forceWindowFrameOpsHandheld)
	{
		newFrameOps.resizable = false;
		newFrameOps.fullscreen = true;
		newFrameOps.changeResolution = false;
		newFrameOps.forceIntegerScaling = false;
	}

	auto currentFrameOpsNorm = currentFrameOps.Normalize();
	auto newFrameOpsNorm = newFrameOps.Normalize();
	auto recreate = !sdl_window ||
	                // Recreate the window when toggling fullscreen, due to occasional issues
	                newFrameOpsNorm.fullscreen       != currentFrameOpsNorm.fullscreen       ||
	                // Also recreate it when enabling resizable windows, to fix bugs on windows,
	                //  see https://github.com/jacob1/The-Powder-Toy/issues/24
	                newFrameOpsNorm.resizable        != currentFrameOpsNorm.resizable        ||
	                newFrameOpsNorm.changeResolution != currentFrameOpsNorm.changeResolution ||
	                newFrameOpsNorm.blurryScaling    != currentFrameOpsNorm.blurryScaling    ||
	                newVsyncHint != vsyncHint;

	if (!(recreate ||
	      newFrameOpsNorm.scale               != currentFrameOpsNorm.scale               ||
	      newFrameOpsNorm.forceIntegerScaling != currentFrameOpsNorm.forceIntegerScaling))
	{
		return;
	}

	auto size = WINDOW * newFrameOpsNorm.scale;
	if (sdl_window && newFrameOpsNorm.resizable)
	{
		SDL_GetWindowSize(sdl_window, &size.X, &size.Y);
	}

	if (recreate)
	{
		if (sdl_texture)
		{
			SDL_DestroyTexture(sdl_texture);
			sdl_texture = nullptr;
		}
		if (sdl_renderer)
		{
			SDL_DestroyRenderer(sdl_renderer);
			sdl_renderer = nullptr;
		}
		if (sdl_window)
		{
			SaveWindowPosition();
			SDL_DestroyWindow(sdl_window);
			sdl_window = nullptr;
		}

		unsigned int flags = 0;
		unsigned int rendererFlags = 0;
		if (newFrameOpsNorm.fullscreen)
		{
			flags = newFrameOpsNorm.changeResolution ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_FULLSCREEN_DESKTOP;
		}
		if (newFrameOpsNorm.resizable)
		{
			flags |= SDL_WINDOW_RESIZABLE;
		}
		if (vsyncHint)
		{
			rendererFlags |= SDL_RENDERER_PRESENTVSYNC;
		}
		sdl_window = SDL_CreateWindow(APPNAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, size.X, size.Y, flags);
		if (!sdl_window)
		{
			fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
			Platform::Exit(-1);
		}
		if constexpr (SET_WINDOW_ICON)
		{
			WindowIcon(sdl_window);
		}
		SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, newFrameOpsNorm.blurryScaling ? "linear" : "nearest");
		sdl_renderer = SDL_CreateRenderer(sdl_window, -1, rendererFlags);
		if (!sdl_renderer)
		{
			fprintf(stderr, "SDL_CreateRenderer failed; available renderers:\n");
			int num = SDL_GetNumRenderDrivers();
			for (int i = 0; i < num; ++i)
			{
				SDL_RendererInfo info;
				SDL_GetRenderDriverInfo(i, &info);
				fprintf(stderr, " - %s\n", info.name);
			}
			Platform::Exit(-1);
		}
		sdl_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
		                                WINDOWW * PIXEL_SCALE, WINDOWH * PIXEL_SCALE);
		if (!sdl_texture)
		{
			fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
			Platform::Exit(-1);
		}
		SDL_RaiseWindow(sdl_window);
		Clipboard::RecreateWindow();
	}
	if (!(newFrameOpsNorm.resizable && SDL_GetWindowFlags(sdl_window) & SDL_WINDOW_MAXIMIZED))
	{
		SDL_SetWindowSize(sdl_window, size.X, size.Y);
		LoadWindowPosition();
	}
	ApplyFpsLimit();
	if (newFrameOpsNorm.fullscreen)
	{
		SDL_RaiseWindow(sdl_window);
	}
	currentFrameOps = newFrameOps;
	vsyncHint = newVsyncHint;
}

static void EventProcess(const SDL_Event &event)
{
	auto &engine = ui::Engine::Ref();
	switch (event.type)
	{
	case SDL_QUIT:
		if (ALLOW_QUIT && (engine.GetFastQuit() || engine.CloseWindow()))
		{
			engine.Exit();
		}
		break;
	case SDL_KEYDOWN:
		if (SDL_GetModState() & KMOD_GUI)
		{
			break;
		}
		if (engine.GetGlobalQuit() && ALLOW_QUIT && !event.key.repeat && event.key.keysym.sym == 'q' && (event.key.keysym.mod&KMOD_CTRL) && !(event.key.keysym.mod&KMOD_ALT))
			engine.ConfirmExit();
		else
			engine.onKeyPress(event.key.keysym.sym, event.key.keysym.scancode, event.key.repeat, event.key.keysym.mod&KMOD_SHIFT, event.key.keysym.mod&KMOD_CTRL, event.key.keysym.mod&KMOD_ALT);
		break;
	case SDL_KEYUP:
		if (SDL_GetModState() & KMOD_GUI)
		{
			break;
		}
		engine.onKeyRelease(event.key.keysym.sym, event.key.keysym.scancode, event.key.repeat, event.key.keysym.mod&KMOD_SHIFT, event.key.keysym.mod&KMOD_CTRL, event.key.keysym.mod&KMOD_ALT);
		break;
	case SDL_TEXTINPUT:
		if (SDL_GetModState() & KMOD_GUI)
		{
			break;
		}
		engine.onTextInput(ByteString(event.text.text).FromUtf8());
		break;
	case SDL_TEXTEDITING:
		if (SDL_GetModState() & KMOD_GUI)
		{
			break;
		}
		engine.onTextEditing(ByteString(event.edit.text).FromUtf8(), event.edit.start);
		break;
	case SDL_MOUSEWHEEL:
	{
		// int x = event.wheel.x;
		int y = event.wheel.y;
		if (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED)
		{
			// x *= -1;
			y *= -1;
		}

		engine.onMouseWheel(mousex, mousey, y); // TODO: pass x?
		break;
	}
	case SDL_MOUSEMOTION:
	{
		auto [gx, gy] = WindowToGame(event.motion.x, event.motion.y);
		mousex = gx;
		mousey = gy;
		engine.onMouseMove(mousex, mousey);
		hasMouseMoved = true;
		break;
	}
	case SDL_DROPFILE:
		engine.onFileDrop(event.drop.file);
		SDL_free(event.drop.file);
		break;
	case SDL_MOUSEBUTTONDOWN:
		// if mouse hasn't moved yet, sdl will send 0,0. We don't want that
		if (hasMouseMoved)
		{
			auto [gx, gy] = WindowToGame(event.button.x, event.button.y);
			mousex = gx;
			mousey = gy;
		}
		mouseButton = event.button.button;
		engine.onMouseDown(mousex, mousey, mouseButton);

		mouseDown = true;
		if constexpr (!DEBUG)
		{
			SDL_CaptureMouse(SDL_TRUE);
		}
		break;
	case SDL_MOUSEBUTTONUP:
		// if mouse hasn't moved yet, sdl will send 0,0. We don't want that
		if (hasMouseMoved)
		{
			auto [gx, gy] = WindowToGame(event.button.x, event.button.y);
			mousex = gx;
			mousey = gy;
		}
		mouseButton = event.button.button;
		engine.onMouseUp(mousex, mousey, mouseButton);

		mouseDown = false;
		if constexpr (!DEBUG)
		{
			SDL_CaptureMouse(SDL_FALSE);
		}
		break;
	case SDL_WINDOWEVENT:
	{
		switch (event.window.event)
		{
		case SDL_WINDOWEVENT_SHOWN:
			if (!calculatedInitialMouse)
			{
				//initial mouse coords, sdl won't tell us this if mouse hasn't moved
				CalculateMousePosition(&mousex, &mousey);
				engine.initialMouse(mousex, mousey);
				engine.onMouseMove(mousex, mousey);
				calculatedInitialMouse = true;
			}
			break;

		case SDL_WINDOWEVENT_DISPLAY_CHANGED:
			UpdateRefreshRate();
			break;
		}
		break;
	}
	}
}

std::optional<uint64_t> EngineProcess()
{
	auto &engine = ui::Engine::Ref();

	{
		auto nowNs = GetNowNs();
		if (clientTickSchedule.HasElapsed(nowNs))
		{
			TickClient();
			clientTickSchedule.SetNow(nowNs);
		}
		clientTickSchedule.Arm(10);
		if (fpsUpdateSchedule.HasElapsed(nowNs))
		{
			engine.SetFps(1e9f / correctedFrameTimeAvg);
			fpsUpdateSchedule.SetNow(nowNs);
		}
		fpsUpdateSchedule.Arm(5);
	}

	if (showLargeScreenDialog)
	{
		showLargeScreenDialog = false;
		LargeScreenDialog();
	}

	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		EventProcess(event);
	}

	std::optional<uint64_t> delay;
	auto nowNs = GetNowNs();
	auto effectiveDrawLimit = engine.GetEffectiveDrawCap();
	auto doDraw = !effectiveDrawLimit || drawSchedule.HasElapsed(nowNs);
	auto fpsLimit = ui::Engine::Ref().GetFpsLimit();
	auto doSimTick = true;
	if (std::holds_alternative<FpsLimitExplicit>(fpsLimit))
	{
		doSimTick = tickSchedule.HasElapsed(nowNs);
	}
	else if (std::holds_alternative<FpsLimitFollowDraw>(fpsLimit))
	{
		doSimTick = doDraw;
	}
	if (doDraw)
	{
		engine.Tick();
	}
	if (doSimTick)
	{
		auto thisContributesToFps = engine.GetContributesToFps();
		if (prevContributesToFps && thisContributesToFps)
		{
			auto correctedFrameTime = tickSchedule.GetFrameTime();
			correctedFrameTimeAvg = correctedFrameTimeAvg + (correctedFrameTime - correctedFrameTimeAvg) * 0.05;
		}
		prevContributesToFps = thisContributesToFps;
		engine.SimTick();
		tickSchedule.SetNow(nowNs);
	}
	if (doDraw)
	{
		// Set up native text overlay before drawing so BlendChar writes to it
		if (sdl_window && engine.g)
		{
			int winW, winH;
			SDL_GetWindowSize(sdl_window, &winW, &winH);
			SDL_Rect gr = ComputeGameRect();
			auto &nt = engine.g->nativeText;
			nt.scaleX = (float)gr.w / WINDOWW;
			nt.scaleY = (float)gr.h / WINDOWH;
			nt.baseX  = gr.x;
			nt.baseY  = gr.y;
			nt.w      = winW;
			nt.h      = winH;
			nt.active = true;
			size_t needed = (size_t)winW * winH;
			if (nt.pixels.size() != needed)
				nt.pixels.resize(needed);
			nt.Clear();
		}

		engine.Draw();
		drawSchedule.SetNow(nowNs);
		SDLSetScreen();
		blit(engine.g->Data(), engine.g);
	}
	if (effectiveDrawLimit)
	{
		delay = drawSchedule.Arm(float(*effectiveDrawLimit)) / UINT64_C(1'000'000);
	}
	if (auto *fpsLimitExplicit = std::get_if<FpsLimitExplicit>(&fpsLimit))
	{
		auto simDelay = tickSchedule.Arm(fpsLimitExplicit->value) / UINT64_C(1'000'000);
		if (delay.has_value() && simDelay < *delay)
		{
			delay = simDelay;
		}
	}
	else if (std::holds_alternative<FpsLimitNone>(fpsLimit))
	{
		delay.reset();
	}
	return delay;
}
