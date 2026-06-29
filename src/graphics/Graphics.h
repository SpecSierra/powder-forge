#pragma once
#include "Icons.h"
#include "gui/interface/Point.h"
#include "common/Plane.h"
#include "SimulationConfig.h"
#include "PixelScale.h"
#include "RasterDrawMethods.h"
#include <vector>

class Graphics: public RasterDrawMethods<Graphics>
{
	// Native-resolution pixel buffer: WINDOWW*PIXEL_SCALE x WINDOWH*PIXEL_SCALE.
	// Initialised in the constructor once PIXEL_SCALE is known.
	PlaneAdapter<std::vector<pixel>> video;

	// Clip rect stays in LOGICAL coordinates (0..WINDOWW, 0..WINDOWH).
	Rect<int> clipRect;

	friend struct RasterDrawMethods<Graphics>;

public:
	// Logical size – used by the UI layout engine.
	Vec2<int> Size() const
	{
		return Vec2<int>{ WINDOWW, WINDOWH };
	}

	// Native (pixel-buffer) size – used when copying the buffer to SDL.
	Vec2<int> NativeSize() const
	{
		return video.Size();
	}

	int GetPixelScale() const
	{
		return PIXEL_SCALE;
	}

	// Native-resolution text overlay for crispy text at any display scale.
	NativeTextOverlay nativeText;
	NativeTextOverlay *GetNativeTextOverlay() { return &nativeText; }

	pixel const *Data() const
	{
		return video.data();
	}

	pixel *Data()
	{
		return video.data();
	}

	VideoBuffer DumpFrame();

	void draw_icon(int x, int y, Icon icon, unsigned char alpha = 255, bool invert = false);

	void Finalise();

	Graphics();

	void SwapClipRect(Rect<int> &);

	Rect<int> GetClipRect() const
	{
		return clipRect;
	}

	ui::Point zoomWindowPosition = { 0, 0 };
	ui::Point zoomScopePosition = { 0, 0 };
	int zoomScopeSize = 32;
	bool zoomEnabled = false;
	int ZFACTOR = 8;
	void RenderZoom();
};
