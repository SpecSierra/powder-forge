#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>

// Singleton that wraps stb_truetype and provides a glyph cache.
// Falls back gracefully to nullptr returns if no font file could be loaded;
// callers then fall back to the legacy bitmap FontReader.
class TrueTypeFont
{
public:
	static TrueTypeFont &Ref();

	bool IsLoaded() const { return loaded; }

	// Target render height in logical pixels.
	static constexpr float TTF_PIXEL_HEIGHT = 10.0f;

	struct Glyph
	{
		std::vector<uint8_t> bitmap; // 8-bit alpha, row-major
		int w = 0, h = 0;           // bitmap dimensions
		int xoff = 0, yoff = 0;     // offset from baseline position
		int advance = 5;             // horizontal advance in pixels
	};

	// Returns nullptr if the codepoint is not in the font or font failed to load.
	// Glyphs are rendered at TTF_PIXEL_HEIGHT * PIXEL_SCALE native pixels.
	const Glyph *GetGlyph(int codepoint);

	// Get a glyph rendered at an arbitrary native pixel height.
	// Used by the native-resolution text overlay for crispy display-size rendering.
	const Glyph *GetGlyphAt(int codepoint, float pixelHeight);

	// Baseline Y offset relative to the position passed to BlendChar.
	// Chosen so glyphs fit within the legacy [pos.Y-2, pos.Y+FONT_H-2] render area.
	int GetBaseline() const { return baseline; }

private:
	TrueTypeFont();
	const Glyph *RasteriseGlyph(int codepoint);
	const Glyph *RasteriseGlyphAt(int codepoint, float pixelHeight);

	bool loaded = false;
	std::vector<uint8_t> fontBuffer;
	void *stbInfo = nullptr; // stbtt_fontinfo*, opaque to avoid leaking stb headers
	float scale = 1.0f;
	int baseline = 8;
	std::unordered_map<int, Glyph> cache;
	// Scaled cache keyed by (codepoint << 16) | roundedPixelHeight
	std::unordered_map<uint64_t, Glyph> scaledCache;
};
