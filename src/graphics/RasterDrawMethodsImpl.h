#include <cmath>
#include <cstring>
#include "common/RasterGeometry.h"
#include "FontReader.h"
#include "PixelScale.h"
#include "TrueTypeFont.h"
#include "VideoBuffer.h"
#include "RasterDrawMethods.h"

#define clipRect() (static_cast<Derived const &>(*this).GetClipRect())

// Low-level helpers: write directly at the given (already-scaled) position.

template<typename Derived, typename V>
static inline void drawPixelUnchecked(RasterDrawMethods<Derived> &self, V Derived::*video, Vec2<int> pos, RGB colour)
{
	(static_cast<Derived &>(self).*video)[pos] = colour.Pack();
}

template<typename Derived, typename V>
static inline void blendPixelUnchecked(RasterDrawMethods<Derived> &self, V Derived::*video, Vec2<int> pos, RGBA colour)
{
	pixel &px = (static_cast<Derived &>(self).*video)[pos];
	px = RGB::Unpack(px).Blend(colour).Pack();
}

template<typename Derived, typename V>
static inline void xorPixelUnchecked(RasterDrawMethods<Derived> &self, V Derived::*video, Vec2<int> pos)
{
	pixel &px = (static_cast<Derived &>(self).*video)[pos];
	auto const c = RGB::Unpack(px);
	if (2 * c.Red + 3 * c.Green + c.Blue < 512)
		px = 0xC0C0C0_rgb .Pack();
	else
		px = 0x404040_rgb .Pack();
}

// ── Single-pixel primitives ───────────────────────────────────────────────────
// Each logical pixel is written as a ps×ps native block.

template<typename Derived>
inline void RasterDrawMethods<Derived>::DrawPixel(Vec2<int> pos, RGB colour)
{
	if (!clipRect().Contains(pos))
		return;
	int ps = static_cast<Derived const &>(*this).GetPixelScale();
	auto native = pos * ps;
	for (int dy = 0; dy < ps; dy++)
		for (int dx = 0; dx < ps; dx++)
			drawPixelUnchecked(*this, &Derived::video, native + Vec2{ dx, dy }, colour);
}

template<typename Derived>
inline void RasterDrawMethods<Derived>::BlendPixel(Vec2<int> pos, RGBA colour)
{
	if (!clipRect().Contains(pos))
		return;
	int ps = static_cast<Derived const &>(*this).GetPixelScale();
	auto native = pos * ps;
	for (int dy = 0; dy < ps; dy++)
		for (int dx = 0; dx < ps; dx++)
			blendPixelUnchecked(*this, &Derived::video, native + Vec2{ dx, dy }, colour);
}

template<typename Derived>
inline void RasterDrawMethods<Derived>::AddPixel(Vec2<int> pos, RGBA colour)
{
	if (!clipRect().Contains(pos))
		return;
	int ps = static_cast<Derived const &>(*this).GetPixelScale();
	auto native = pos * ps;
	auto &video = static_cast<Derived &>(*this).video;
	for (int dy = 0; dy < ps; dy++)
		for (int dx = 0; dx < ps; dx++) {
			pixel &px = video[native + Vec2{ dx, dy }];
			px = RGB::Unpack(px).Add(colour).Pack();
		}
}

template<typename Derived>
inline void RasterDrawMethods<Derived>::AddFirePixel(Vec2<int> pos, RGB colour, int fireAlpha)
{
	if (!clipRect().Contains(pos))
		return;
	int ps = static_cast<Derived const &>(*this).GetPixelScale();
	auto native = pos * ps;
	auto &video = static_cast<Derived &>(*this).video;
	for (int dy = 0; dy < ps; dy++)
		for (int dx = 0; dx < ps; dx++) {
			pixel &px = video[native + Vec2{ dx, dy }];
			px = RGB::Unpack(px).AddFire(colour, fireAlpha).Pack();
		}
}

template<typename Derived>
inline void RasterDrawMethods<Derived>::XorPixel(Vec2<int> pos)
{
	if (!clipRect().Contains(pos))
		return;
	int ps = static_cast<Derived const &>(*this).GetPixelScale();
	auto native = pos * ps;
	for (int dy = 0; dy < ps; dy++)
		for (int dx = 0; dx < ps; dx++)
			xorPixelUnchecked(*this, &Derived::video, native + Vec2{ dx, dy });
}

// ── Line / rect outlines ──────────────────────────────────────────────────────
// These delegate to the single-pixel primitives, which handle scaling.

template<typename Derived>
void RasterDrawMethods<Derived>::DrawLine(Vec2<int> pos1, Vec2<int> pos2, RGB colour)
{
	RasterizeLine<false>(pos1, pos2, [this, colour](Vec2<int> pos) {
		DrawPixel(pos, colour);
	});
}

template<typename Derived>
void RasterDrawMethods<Derived>::BlendLine(Vec2<int> pos1, Vec2<int> pos2, RGBA colour)
{
	RasterizeLine<false>(pos1, pos2, [this, colour](Vec2<int> pos) {
		BlendPixel(pos, colour);
	});
}

template<typename Derived>
void RasterDrawMethods<Derived>::AddLine(Vec2<int> pos1, Vec2<int> pos2, RGBA colour)
{
	RasterizeLine<false>(pos1, pos2, [this, colour](Vec2<int> pos) {
		AddPixel(pos, colour);
	});
}

template<typename Derived>
void RasterDrawMethods<Derived>::XorLine(Vec2<int> pos1, Vec2<int> pos2)
{
	RasterizeLine<false>(pos1, pos2, [this](Vec2<int> pos) {
		XorPixel(pos);
	});
}

template<typename Derived>
void RasterDrawMethods<Derived>::DrawRect(Rect<int> rect, RGB colour)
{
	RasterizeRect(rect, [this, colour](Vec2<int> pos) {
		DrawPixel(pos, colour);
	});
}

template<typename Derived>
void RasterDrawMethods<Derived>::BlendRect(Rect<int> rect, RGBA colour)
{
	RasterizeRect(rect, [this, colour](Vec2<int> pos) {
		BlendPixel(pos, colour);
	});
}

template<typename Derived>
void RasterDrawMethods<Derived>::XorDottedRect(Rect<int> rect)
{
	RasterizeDottedRect(rect, [this](Vec2<int> pos) {
		XorPixel(pos);
	});
}

// ── Filled rectangles ─────────────────────────────────────────────────────────

template<typename Derived>
void RasterDrawMethods<Derived>::DrawFilledRect(Rect<int> rect, RGB colour)
{
	rect &= clipRect();
	if (!rect)
		return;
	int ps = static_cast<Derived const &>(*this).GetPixelScale();
	pixel packed = colour.Pack();
	auto &video = static_cast<Derived &>(*this).video;
	for (int y = rect.pos.Y; y < rect.pos.Y + rect.size.Y; y++)
		for (int dy = 0; dy < ps; dy++)
			std::fill_n(video.RowIterator(Vec2(rect.pos.X * ps, y * ps + dy)),
			            rect.size.X * ps, packed);
}

template<typename Derived>
void RasterDrawMethods<Derived>::BlendFilledRect(Rect<int> rect, RGBA colour)
{
	int ps = static_cast<Derived const &>(*this).GetPixelScale();
	for (auto pos : rect & clipRect()) {
		auto native = pos * ps;
		for (int dy = 0; dy < ps; dy++)
			for (int dx = 0; dx < ps; dx++)
				blendPixelUnchecked(*this, &Derived::video, native + Vec2{ dx, dy }, colour);
	}
}

// ── Ellipses ──────────────────────────────────────────────────────────────────

template<typename Derived>
void RasterDrawMethods<Derived>::BlendEllipse(Vec2<int> center, Vec2<int> size, RGBA colour)
{
	RasterizeEllipsePoints(Vec2(float(size.X * size.X), float(size.Y * size.Y)), [this, center, colour](Vec2<int> delta) {
		BlendPixel(center + delta, colour);
	});
}

template<typename Derived>
void RasterDrawMethods<Derived>::BlendFilledEllipse(Vec2<int> center, Vec2<int> size, RGBA colour)
{
	int ps = static_cast<Derived const &>(*this).GetPixelScale();
	RasterizeEllipseRows(Vec2(float(size.X * size.X), float(size.Y * size.Y)), [this, center, colour, ps](int xLim, int dy) {
		for (auto pos : clipRect() & RectBetween(center + Vec2(-xLim, dy), center + Vec2(xLim, dy))) {
			auto native = pos * ps;
			for (int ny = 0; ny < ps; ny++)
				for (int nx = 0; nx < ps; nx++)
					blendPixelUnchecked(*this, &Derived::video, native + Vec2{ nx, ny }, colour);
		}
	});
}

// ── Image blitting ────────────────────────────────────────────────────────────
// At ps=1 (Renderer, VideoBuffer) these are direct copies.
// At ps>1 (Graphics) each source pixel expands to a ps×ps native block.

template<typename Derived>
void RasterDrawMethods<Derived>::BlendImage(pixel const *data, uint8_t alpha, Rect<int> rect)
{
	BlendImage(data, alpha, rect, rect.size.X);
}

template<typename Derived>
void RasterDrawMethods<Derived>::BlendImage(pixel const *data, uint8_t alpha, Rect<int> rect, size_t rowStride)
{
	auto origin = rect.pos;
	rect &= clipRect();
	if (!rect)
		return;
	int ps = static_cast<Derived const &>(*this).GetPixelScale();
	auto &video = static_cast<Derived &>(*this).video;

	if (ps == 1)
	{
		if (alpha == 0xFF)
		{
			for (int y = rect.pos.Y; y < rect.pos.Y + rect.size.Y; y++)
				std::copy_n(
					data + (rect.pos.X - origin.X) + (y - origin.Y) * rowStride,
					rect.size.X,
					video.RowIterator(Vec2(rect.pos.X, y))
				);
		}
		else
		{
			for (auto pos : rect)
			{
				pixel const px = data[(pos.X - origin.X) + (pos.Y - origin.Y) * rowStride];
				blendPixelUnchecked(*this, &Derived::video, pos, RGB::Unpack(px).WithAlpha(alpha));
			}
		}
	}
	else // ps > 1: expand each source pixel to a ps×ps native block
	{
		for (int y = rect.pos.Y; y < rect.pos.Y + rect.size.Y; y++)
		{
			for (int x = rect.pos.X; x < rect.pos.X + rect.size.X; x++)
			{
				pixel const px = data[(x - origin.X) + (y - origin.Y) * rowStride];
				if (alpha == 0xFF)
				{
					for (int dy = 0; dy < ps; dy++)
						std::fill_n(video.RowIterator(Vec2(x * ps, y * ps + dy)), ps, px);
				}
				else
				{
					RGBA c = RGB::Unpack(px).WithAlpha(alpha);
					for (int dy = 0; dy < ps; dy++)
						for (int dx = 0; dx < ps; dx++)
							blendPixelUnchecked(*this, &Derived::video, Vec2(x * ps + dx, y * ps + dy), c);
				}
			}
		}
	}
}

template<typename Derived>
void RasterDrawMethods<Derived>::XorImage(unsigned char const *data, Rect<int> rect)
{
	XorImage(data, rect, rect.size.X);
}

template<typename Derived>
void RasterDrawMethods<Derived>::XorImage(unsigned char const *data, Rect<int> rect, size_t rowStride)
{
	auto origin = rect.pos;
	rect &= clipRect();
	int ps = static_cast<Derived const &>(*this).GetPixelScale();
	for (auto pos : rect)
	{
		if (data[(pos.X - origin.X) + (pos.Y - origin.Y) * rowStride])
		{
			auto native = pos * ps;
			for (int dy = 0; dy < ps; dy++)
				for (int dx = 0; dx < ps; dx++)
					xorPixelUnchecked(*this, &Derived::video, native + Vec2{ dx, dy });
		}
	}
}

template<typename Derived>
void RasterDrawMethods<Derived>::BlendRGBAImage(pixel_rgba const *data, Rect<int> rect)
{
	BlendRGBAImage(data, rect, rect.size.X);
}

template<typename Derived>
void RasterDrawMethods<Derived>::BlendRGBAImage(pixel_rgba const *data, Rect<int> rect, size_t rowStride)
{
	auto origin = rect.pos;
	rect &= clipRect();
	int ps = static_cast<Derived const &>(*this).GetPixelScale();
	for (auto pos : rect)
	{
		pixel const px = data[(pos.X - origin.X) + (pos.Y - origin.Y) * rowStride];
		RGBA c = RGBA::Unpack(px);
		auto native = pos * ps;
		for (int dy = 0; dy < ps; dy++)
			for (int dx = 0; dx < ps; dx++)
				blendPixelUnchecked(*this, &Derived::video, native + Vec2{ dx, dy }, c);
	}
}

// ── Native-pixel write (for TrueType glyph placement) ────────────────────────

template<typename Derived>
void RasterDrawMethods<Derived>::BlendNativePixel(Vec2<int> nativePos, RGBA colour)
{
	auto &video = static_cast<Derived &>(*this).video;
	if (nativePos.X < 0 || nativePos.Y < 0 ||
	    nativePos.X >= video.Size().X || nativePos.Y >= video.Size().Y)
		return;
	// Respect the logical clip rect (convert to native coords).
	int ps = static_cast<Derived const &>(*this).GetPixelScale();
	if (ps > 1)
	{
		auto const &cr = clipRect();
		if (nativePos.X <  cr.pos.X  * ps || nativePos.Y <  cr.pos.Y  * ps ||
		    nativePos.X >= (cr.pos.X + cr.size.X) * ps ||
		    nativePos.Y >= (cr.pos.Y + cr.size.Y) * ps)
			return;
	}
	pixel &px = video[nativePos];
	px = RGB::Unpack(px).Blend(colour).Pack();
}

// ── Text rendering ────────────────────────────────────────────────────────────

template<typename Derived>
int RasterDrawMethods<Derived>::BlendChar(Vec2<int> pos, String::value_type ch, RGBA colour)
{
	int ps = static_cast<Derived const &>(*this).GetPixelScale();

	// PUA icons and low-scale (ps<2) contexts always use the bitmap font.
	if (ch >= 0xE000 || !TrueTypeFont::Ref().IsLoaded() || ps < 2)
	{
		FontReader reader(ch);
		int bw = reader.GetWidth();
		if (ps <= 1)
		{
			auto const rect = RectSized(Vec2(0, -2), Vec2(bw, FONT_H));
			for (auto off : rect.template Range<TOP_TO_BOTTOM, LEFT_TO_RIGHT>())
				BlendPixel(pos + off, colour.NoAlpha().WithAlpha(reader.NextPixel() * colour.Alpha / 3));
		}
		else
		{
			// Bilinear upscale: render each bitmap icon at native resolution
			// (bw*ps × FONT_H*ps) so icons are as crisp as TrueType text.
			constexpr int MAX_BW = 32;
			uint8_t src[MAX_BW * FONT_H] = {};
			int bwc = std::min(bw, MAX_BW);
			for (int by = 0; by < FONT_H; by++)
				for (int bx = 0; bx < bw; bx++)
				{
					uint8_t v = (uint8_t)reader.NextPixel();
					if (bx < bwc)
						src[by * bwc + bx] = v;
				}
			Vec2<int> nativeBase = (pos + Vec2{0, -2}) * ps;
			RGB const rgb = colour.NoAlpha();
			for (int ny = 0; ny < FONT_H * ps; ny++)
			{
				float fy = (float)ny / ps;
				int by0 = (int)fy, by1 = std::min(by0 + 1, FONT_H - 1);
				float ty = fy - by0;
				for (int nx = 0; nx < bwc * ps; nx++)
				{
					float fx = (float)nx / ps;
					int bx0 = (int)fx, bx1 = std::min(bx0 + 1, bwc - 1);
					float tx = fx - bx0;
					float a = src[by0 * bwc + bx0] * (1.0f - tx) * (1.0f - ty)
					        + src[by0 * bwc + bx1] * tx            * (1.0f - ty)
					        + src[by1 * bwc + bx0] * (1.0f - tx) * ty
					        + src[by1 * bwc + bx1] * tx            * ty;
					auto alpha = (uint8_t)(a * colour.Alpha / 3.0f + 0.5f);
					if (alpha)
						BlendNativePixel(nativeBase + Vec2{nx, ny}, rgb.WithAlpha(alpha));
				}
			}
		}
		return bw;
	}

	// Native-resolution overlay path: render glyph directly at window pixel size.
	// Only active when Graphics::nativeText is set up (not VideoBuffer/Renderer).
	auto *nt = static_cast<Derived &>(*this).GetNativeTextOverlay();
	if (nt && nt->active && TrueTypeFont::Ref().IsLoaded())
	{
		float pixH = TrueTypeFont::TTF_PIXEL_HEIGHT * nt->scaleY;
		const auto *glyph = TrueTypeFont::Ref().GetGlyphAt(int(ch), pixH);
		if (!glyph)
		{
			FontReader reader(ch);
			return reader.GetWidth();
		}
		if (glyph->bitmap.empty())
			return glyph->advance;

		RGB const rgb = colour.NoAlpha();
		int originX = (int)(nt->baseX + pos.X * nt->scaleX + 0.5f);
		int originY = (int)(nt->baseY + pos.Y * nt->scaleY + 0.5f);

		// Map logical clip rect to window pixel bounds
		auto const &cr = clipRect();
		int cx0 = (int)(nt->baseX + cr.pos.X * nt->scaleX);
		int cy0 = (int)(nt->baseY + cr.pos.Y * nt->scaleY);
		int cx1 = (int)(nt->baseX + (cr.pos.X + cr.size.X) * nt->scaleX);
		int cy1 = (int)(nt->baseY + (cr.pos.Y + cr.size.Y) * nt->scaleY);

		for (int gy = 0; gy < glyph->h; ++gy)
		{
			for (int gx = 0; gx < glyph->w; ++gx)
			{
				uint8_t a8 = glyph->bitmap[gy * glyph->w + gx];
				if (!a8) continue;
				uint32_t a = (uint32_t(a8) * colour.Alpha) / 255u;
				if (!a) continue;
				int px = originX + glyph->xoff + gx;
				int py = originY + glyph->yoff + gy;
				if (px < cx0 || py < cy0 || px >= cx1 || py >= cy1) continue;
				if (px < 0 || py < 0 || px >= nt->w || py >= nt->h) continue;

				// SRC over DST alpha composite
				uint32_t &dst = nt->pixels[py * nt->w + px];
				uint32_t da  = (dst >> 24) & 0xFFu;
				uint32_t dr  = (dst >> 16) & 0xFFu;
				uint32_t dg  = (dst >>  8) & 0xFFu;
				uint32_t db  = (dst      ) & 0xFFu;
				uint32_t inv = 255u - a;
				uint32_t oa  = a + da * inv / 255u;
				uint32_t or_ = (uint32_t(rgb.Red)   * a + dr * da * inv / 255u) / (oa ? oa : 1u);
				uint32_t og  = (uint32_t(rgb.Green) * a + dg * da * inv / 255u) / (oa ? oa : 1u);
				uint32_t ob  = (uint32_t(rgb.Blue)  * a + db * da * inv / 255u) / (oa ? oa : 1u);
				dst = (oa << 24) | (or_ << 16) | (og << 8) | ob;
			}
		}
		return glyph->advance;
	}

	// Normal Unicode → TrueType renderer.
	// Glyphs are pre-rasterised at TTF_PIXEL_HEIGHT*PIXEL_SCALE native pixels;
	// place them at exact native coordinates (no block expansion).
	auto &ttf = TrueTypeFont::Ref();
	const auto *glyph = ttf.GetGlyph(int(ch));
	if (!glyph)
	{
		FontReader reader(ch);
		auto const rect = RectSized(Vec2(0, -2), Vec2(reader.GetWidth(), FONT_H));
		for (auto off : rect.template Range<TOP_TO_BOTTOM, LEFT_TO_RIGHT>())
			BlendPixel(pos + off, colour.NoAlpha().WithAlpha(reader.NextPixel() * colour.Alpha / 3));
		return reader.GetWidth();
	}

	if (glyph->bitmap.empty())
		return glyph->advance;

	RGB const rgb = colour.NoAlpha();
	Vec2<int> const nativeOrigin = pos * ps;
	for (int y = 0; y < glyph->h; ++y)
		for (int x = 0; x < glyph->w; ++x)
		{
			uint8_t a = glyph->bitmap[y * glyph->w + x];
			if (a)
				BlendNativePixel(
					nativeOrigin + Vec2<int>{ x + glyph->xoff, y + glyph->yoff },
					rgb.WithAlpha(uint8_t(uint32_t(a) * colour.Alpha / 255)));
		}
	return glyph->advance; // logical advance
}

template<typename Derived>
int RasterDrawMethods<Derived>::AddChar(Vec2<int> pos, String::value_type ch, RGBA colour)
{
	FontReader reader(ch);
	RGB const c = colour.NoAlpha();
	auto const rect = RectSized(Vec2(0, -2), Vec2(reader.GetWidth(), FONT_H));
	for (auto off : rect.template Range<TOP_TO_BOTTOM, LEFT_TO_RIGHT>())
		AddPixel(pos + off, c.WithAlpha(reader.NextPixel() * colour.Alpha / 3));
	return reader.GetWidth();
}

template<typename Derived>
Vec2<int> RasterDrawMethods<Derived>::BlendText(Vec2<int> orig_pos, String const &str, RGBA orig_colour)
{
	bool underline = false;
	bool invert = false;
	RGB colour = orig_colour.NoAlpha();
	uint8_t alpha = orig_colour.Alpha;
	Vec2<int> pos = orig_pos;
	for (size_t i = 0; i < str.length(); i++)
	{
		if (str[i] == '\n')
		{
			pos.X = orig_pos.X;
			pos.Y += FONT_H;
		}
		else if (str[i] == '\x0F')
		{
			if (str.length() <= i + 3)
				break;
			colour.Red = str[i + 1];
			colour.Green = str[i + 2];
			colour.Blue = str[i + 3];
			i += 3;
		}
		else if (str[i] == '\x0E')
		{
			colour = orig_colour.NoAlpha();
		}
		else if (str[i] == '\x01')
		{
			invert = !invert;
			colour = colour.Inverse();
		}
		else if (str[i] == '\b')
		{
			if (str.length() <= i + 1)
				break;
			bool colourCode = true;
			switch (str[i + 1])
			{
			case 'U': underline = !underline; colourCode = false; break;
			case 'w': colour = 0xFFFFFF_rgb; break;
			case 'g': colour = 0xC0C0C0_rgb; break;
			case 'o': colour = 0xFFD820_rgb; break;
			case 'r': colour = 0xFF0000_rgb; break;
			case 'l': colour = 0xFF4B4B_rgb; break;
			case 'b': colour = 0x0000FF_rgb; break;
			case 't': colour = 0x20AAFF_rgb; break;
			case 'u': colour = 0x9353D3_rgb; break;
			}
			if (colourCode && invert)
				colour = colour.Inverse();
			i++;
		}
		else
		{
			int dx = BlendChar(pos, str[i], colour.WithAlpha(alpha));
			if (underline)
				for (int i = 0; i < dx; i++)
					BlendPixel(pos + Vec2(i, FONT_H), colour.WithAlpha(alpha));
			pos.X += dx;
		}
	}
	return pos - orig_pos;
}

template<typename Derived>
Vec2<int> RasterDrawMethods<Derived>::BlendTextOutline(Vec2<int> pos, String const &str, RGBA colour)
{
	BlendText(pos + Vec2(-1, -1), str, 0x000000_rgb .WithAlpha(0x78));
	BlendText(pos + Vec2(-1, +1), str, 0x000000_rgb .WithAlpha(0x78));
	BlendText(pos + Vec2(+1, -1), str, 0x000000_rgb .WithAlpha(0x78));
	BlendText(pos + Vec2(+1, +1), str, 0x000000_rgb .WithAlpha(0x78));

	return BlendText(pos, str, colour);
}

template<typename Derived>
void RasterDrawMethods<Derived>::Clear()
{
	auto &video = static_cast<Derived &>(*this).video;
	std::fill_n(video.data(), video.Size().X * video.Size().Y, 0x000000_rgb .Pack());
}

template<typename Derived>
int RasterDrawMethods<Derived>::CharWidth(String::value_type ch)
{
	if (ch >= 0xE000 || !TrueTypeFont::Ref().IsLoaded() || PIXEL_SCALE < 2)
		return FontReader(ch).GetWidth();

	const auto *glyph = TrueTypeFont::Ref().GetGlyph(int(ch));
	if (!glyph)
		return FontReader(ch).GetWidth();
	return glyph->advance; // logical
}

template<typename Derived>
Vec2<int> RasterDrawMethods<Derived>::TextSize(String const &str)
{
	Vec2<int> size = Vec2(0, FONT_H - 2);
	int curX = 0;
	for (size_t i = 0; i < str.length(); i++)
	{
		if (str[i] == '\n')
		{
			size.X = std::max(curX, size.X);
			size.Y += FONT_H;
			curX = 0;
		}
		else if (str[i] == '\x0F')
		{
			if (str.length() <= i + 3)
				break;
			i += 3;
		}
		else if (str[i] == '\x0E')
			continue;
		else if (str[i] == '\x01')
			continue;
		else if (str[i] == '\b')
		{
			if (str.length() <= i + 1)
				break;
			i++;
		}
		else
			curX += CharWidth(str[i]);
	}
	size.X = std::max(curX, size.X);
	return size;
}

template<typename Derived>
String::const_iterator RasterDrawMethods<Derived>::TextFit(String const &str, int width)
{
	int curX = 0;
	for (size_t i = 0; i < str.length(); i++)
	{
		if (str[i] == '\n')
			curX = 0;
		else if (str[i] == '\x0F')
		{
			if (str.length() <= i + 3)
				break;
			i += 3;
		}
		else if (str[i] == '\x0E')
			continue;
		else if (str[i] == '\x01')
			continue;
		else if (str[i] == '\b')
		{
			if (str.length() <= i + 1)
				break;
			i++;
		}
		else
		{
			int dx = CharWidth(str[i]);
			if (curX + dx / 2 >= width)
				return str.begin() + i;
			curX += dx;
		}
	}
	return str.end();
}

#undef clipRect
