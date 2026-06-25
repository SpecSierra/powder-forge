#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

#include "TrueTypeFont.h"
#include "FontReader.h" // for FONT_H

#include <cstdio>
#include <cmath>
#include <array>
#include <stdexcept>

// ── Font file search paths ────────────────────────────────────────────────────
static constexpr std::array<const char *, 12> FONT_PATHS = {
	// Linux – Ubuntu / Debian
	"/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf",
	"/usr/share/fonts/ubuntu/Ubuntu-R.ttf",
	// Linux – DejaVu fallback
	"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
	"/usr/share/fonts/dejavu/DejaVuSans.ttf",
	// macOS
	"/System/Library/Fonts/SFNS.ttf",
	"/Library/Fonts/Arial.ttf",
	// Windows
	"C:\\Windows\\Fonts\\segoeui.ttf",
	"C:\\Windows\\Fonts\\arial.ttf",
	// Generic fallbacks
	"/usr/share/fonts/TTF/DejaVuSans.ttf",
	"/usr/local/share/fonts/DejaVuSans.ttf",
	nullptr,
	nullptr
};

// ── Target render size ────────────────────────────────────────────────────────
// At 10 px total height (ascent+descent), glyphs sit comfortably within the
// legacy 12-px line cell (FONT_H=12, render area [pos.Y-2, pos.Y+FONT_H-2]).
static constexpr float TTF_PIXEL_HEIGHT = 10.0f;

// ─────────────────────────────────────────────────────────────────────────────

TrueTypeFont &TrueTypeFont::Ref()
{
	static TrueTypeFont instance;
	return instance;
}

TrueTypeFont::TrueTypeFont()
{
	// Try each search path until one works.
	for (auto *path : FONT_PATHS)
	{
		if (!path) break;

		FILE *f = fopen(path, "rb");
		if (!f) continue;

		fseek(f, 0, SEEK_END);
		long sz = ftell(f);
		rewind(f);
		if (sz <= 0) { fclose(f); continue; }

		fontBuffer.resize(size_t(sz));
		bool ok = fread(fontBuffer.data(), 1, size_t(sz), f) == size_t(sz);
		fclose(f);
		if (!ok) { fontBuffer.clear(); continue; }

		auto *info = new stbtt_fontinfo;
		if (!stbtt_InitFont(info, fontBuffer.data(), 0))
		{
			delete info;
			fontBuffer.clear();
			continue;
		}

		stbInfo = info;
		scale = stbtt_ScaleForPixelHeight(info, TTF_PIXEL_HEIGHT);

		// Compute baseline so the full glyph height fits within FONT_H.
		// Render area (legacy compat) = [pos.Y - 2, pos.Y + FONT_H - 2].
		// We want: pos.Y + baseline - ascent_px >= pos.Y - 2
		//       ↔  baseline = ascent_px - 2
		// Then: pos.Y + baseline + |descent_px| ≤ pos.Y + FONT_H - 2 is usually satisfied.
		int ascent_raw = 0, descent_raw = 0, lineGap_raw = 0;
		stbtt_GetFontVMetrics(info, &ascent_raw, &descent_raw, &lineGap_raw);
		int ascent_px  = (int)std::ceil(ascent_raw  * scale);
		baseline = ascent_px - 2; // offset from pos.Y

		loaded = true;
		break;
	}
}

const TrueTypeFont::Glyph *TrueTypeFont::GetGlyph(int codepoint)
{
	if (!loaded) return nullptr;

	auto it = cache.find(codepoint);
	if (it != cache.end())
		return it->second.bitmap.empty() ? nullptr : &it->second;

	return RasteriseGlyph(codepoint);
}

const TrueTypeFont::Glyph *TrueTypeFont::RasteriseGlyph(int codepoint)
{
	auto *info = reinterpret_cast<stbtt_fontinfo *>(stbInfo);

	int advance_raw = 0, lsb_raw = 0;
	stbtt_GetCodepointHMetrics(info, codepoint, &advance_raw, &lsb_raw);

	// Check the codepoint is in the font.
	int gi = stbtt_FindGlyphIndex(info, codepoint);
	if (gi == 0 && codepoint != 0)
	{
		cache[codepoint] = Glyph{}; // mark as "not available"
		return nullptr;
	}

	Glyph g;
	g.advance = (int)(advance_raw * scale + 0.5f);

	int w = 0, h = 0, xoff = 0, yoff = 0;
	uint8_t *bmp = stbtt_GetCodepointBitmap(info, scale, scale, codepoint, &w, &h, &xoff, &yoff);

	if (bmp && w > 0 && h > 0)
	{
		g.w = w;
		g.h = h;
		g.xoff = xoff;
		// yoff from stb is relative to baseline (negative = above baseline).
		// We convert to be relative to pos.Y: yoff_abs = baseline + yoff.
		g.yoff = baseline + yoff;
		g.bitmap.assign(bmp, bmp + w * h);
		stbtt_FreeBitmap(bmp, nullptr);
	}
	else
	{
		if (bmp) stbtt_FreeBitmap(bmp, nullptr);
		// Space or zero-width glyph: store entry with empty bitmap, valid advance.
		g.bitmap.clear();
	}

	cache[codepoint] = std::move(g);
	return g.bitmap.empty() ? nullptr : &cache[codepoint];
}
