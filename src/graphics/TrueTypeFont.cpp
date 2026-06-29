#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

#include "TrueTypeFont.h"
#include "PixelScale.h"
#include "FontReader.h" // for FONT_H

#include <cstdio>
#include <cmath>
#include <algorithm>
#include <array>

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

// Supersampling factor.  Render at SS× then box-downsample SS:1.
// 4× gives 16 samples per output pixel, producing smooth AA at the small
// logical sizes the game uses (10 px logical = 20 px native at PIXEL_SCALE=2).
// No S-curve is applied: box-averaged values look good both displayed 1:1
// in native builds and CSS-bilinear-scaled in the web build.
static constexpr int SS = 4;

// ─────────────────────────────────────────────────────────────────────────────

TrueTypeFont &TrueTypeFont::Ref()
{
	static TrueTypeFont instance;
	return instance;
}

TrueTypeFont::TrueTypeFont()
{
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
		scale = stbtt_ScaleForPixelHeight(info, TTF_PIXEL_HEIGHT * PIXEL_SCALE);

		int ascent_raw = 0, descent_raw = 0, lineGap_raw = 0;
		stbtt_GetFontVMetrics(info, &ascent_raw, &descent_raw, &lineGap_raw);
		int ascent_px = (int)std::ceil(ascent_raw * scale);
		baseline = ascent_px - 2 * PIXEL_SCALE;

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

// Render at SS× then box-downsample SS:1.
// Returns the final bitmap; sets out_bw/out_bh to output dimensions
// and out_ix0/out_iy0 to the (scaled-up then divided-by-SS) bitmap box origin.
static std::vector<uint8_t> Supersample(
	stbtt_fontinfo *info, int codepoint, float scaleBase,
	int &out_bw, int &out_bh, int &out_ix0, int &out_iy0)
{
	float scaleUp = scaleBase * SS;

	int ix0, iy0, ix1, iy1;
	stbtt_GetCodepointBitmapBox(info, codepoint, scaleUp, scaleUp,
	                            &ix0, &iy0, &ix1, &iy1);

	int bwUp = ix1 - ix0;
	int bhUp = iy1 - iy0;
	out_ix0 = ix0;
	out_iy0 = iy0;

	if (bwUp <= 0 || bhUp <= 0)
	{
		out_bw = out_bh = 0;
		return {};
	}

	std::vector<uint8_t> bmpUp(bwUp * bhUp, 0);
	stbtt_MakeCodepointBitmapSubpixel(info,
	                                  bmpUp.data(),
	                                  bwUp, bhUp, bwUp,
	                                  scaleUp, scaleUp,
	                                  0.0f, 0.0f,
	                                  codepoint);

	out_bw = (bwUp + SS - 1) / SS;
	out_bh = (bhUp + SS - 1) / SS;
	std::vector<uint8_t> bmp(out_bw * out_bh, 0);

	for (int y = 0; y < out_bh; ++y)
		for (int x = 0; x < out_bw; ++x)
		{
			int sx = x * SS, sy = y * SS;
			int sum = 0, count = 0;
			for (int dy = 0; dy < SS && sy + dy < bhUp; ++dy)
				for (int dx = 0; dx < SS && sx + dx < bwUp; ++dx)
				{
					sum += bmpUp[(sy + dy) * bwUp + (sx + dx)];
					++count;
				}
			bmp[y * out_bw + x] = count ? uint8_t(sum / count) : 0;
		}

	return bmp;
}

const TrueTypeFont::Glyph *TrueTypeFont::RasteriseGlyph(int codepoint)
{
	auto *info = reinterpret_cast<stbtt_fontinfo *>(stbInfo);

	int advance_raw = 0, lsb_raw = 0;
	stbtt_GetCodepointHMetrics(info, codepoint, &advance_raw, &lsb_raw);

	int gi = stbtt_FindGlyphIndex(info, codepoint);
	if (gi == 0 && codepoint != 0)
	{
		cache[codepoint] = Glyph{};
		return nullptr;
	}

	Glyph g;
	g.advance = (int)(advance_raw * scale / PIXEL_SCALE + 0.5f);

	int bw, bh, ix0, iy0;
	auto bmp = Supersample(info, codepoint, scale, bw, bh, ix0, iy0);

	if (!bmp.empty())
	{
		g.w    = bw;
		g.h    = bh;
		g.xoff = ix0 / SS;
		g.yoff = baseline + iy0 / SS;
		g.bitmap = std::move(bmp);
	}

	cache[codepoint] = std::move(g);
	return cache[codepoint].bitmap.empty() ? nullptr : &cache[codepoint];
}

const TrueTypeFont::Glyph *TrueTypeFont::GetGlyphAt(int codepoint, float pixelHeight)
{
	if (!loaded) return nullptr;

	uint64_t key = ((uint64_t)(uint32_t)codepoint << 16) | (uint64_t)((int)std::round(pixelHeight));
	auto it = scaledCache.find(key);
	if (it != scaledCache.end())
		return it->second.bitmap.empty() ? nullptr : &it->second;

	return RasteriseGlyphAt(codepoint, pixelHeight);
}

const TrueTypeFont::Glyph *TrueTypeFont::RasteriseGlyphAt(int codepoint, float pixelHeight)
{
	auto *info = reinterpret_cast<stbtt_fontinfo *>(stbInfo);

	int advance_raw = 0, lsb_raw = 0;
	stbtt_GetCodepointHMetrics(info, codepoint, &advance_raw, &lsb_raw);

	int gi = stbtt_FindGlyphIndex(info, codepoint);
	if (gi == 0 && codepoint != 0)
	{
		uint64_t key = ((uint64_t)(uint32_t)codepoint << 16) | (uint64_t)((int)std::round(pixelHeight));
		scaledCache[key] = Glyph{};
		return nullptr;
	}

	float scaleAt = stbtt_ScaleForPixelHeight(info, pixelHeight);

	Glyph g;
	g.advance = (int)(advance_raw * scaleAt * TTF_PIXEL_HEIGHT / pixelHeight + 0.5f);

	int bw, bh, ix0, iy0;
	auto bmp = Supersample(info, codepoint, scaleAt, bw, bh, ix0, iy0);

	if (!bmp.empty())
	{
		int ascent_raw = 0;
		stbtt_GetFontVMetrics(info, &ascent_raw, nullptr, nullptr);
		int ascent_px  = (int)std::ceil(ascent_raw * scaleAt);
		int baselineAt = ascent_px - (int)std::round(2.0f * pixelHeight / TTF_PIXEL_HEIGHT);

		g.w    = bw;
		g.h    = bh;
		g.xoff = ix0 / SS;
		g.yoff = baselineAt + iy0 / SS;
		g.bitmap = std::move(bmp);
	}

	uint64_t key = ((uint64_t)(uint32_t)codepoint << 16) | (uint64_t)((int)std::round(pixelHeight));
	scaledCache[key] = std::move(g);
	return scaledCache[key].bitmap.empty() ? nullptr : &scaledCache[key];
}
