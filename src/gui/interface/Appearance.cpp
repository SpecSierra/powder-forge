#include "Appearance.h"
#include "graphics/Graphics.h"
#include "graphics/VideoBuffer.h"
#include <iostream>

namespace ui
{
	// Catppuccin Mocha dark palette
	Appearance::Appearance():
		texture(nullptr),

		VerticalAlign(AlignMiddle),
		HorizontalAlign(AlignCentre),

		BackgroundHover(0x313244_rgb .WithAlpha(0xFF)),    // Surface0: elevated hover
		BackgroundInactive(0x1E1E2E_rgb .WithAlpha(0xFF)), // Base: dark button bg
		BackgroundActive(0x89B4FA_rgb .WithAlpha(0xFF)),   // Blue accent when active
		BackgroundDisabled(0x181825_rgb .WithAlpha(0xFF)), // Mantle: darker disabled

		TextHover(0xFFFFFF_rgb .WithAlpha(0xFF)),          // White on hover
		TextInactive(0xBAC2DE_rgb .WithAlpha(0xFF)),       // Subtext1: normal text
		TextActive(0x1E1E2E_rgb .WithAlpha(0xFF)),         // Dark on active bg
		TextDisabled(0x45475A_rgb .WithAlpha(0xFF)),       // Surface1: muted disabled

		BorderHover(0x89B4FA_rgb .WithAlpha(0xFF)),        // Blue accent on hover
		BorderInactive(0x45475A_rgb .WithAlpha(0xFF)),     // Surface1: subtle border
		BorderActive(0xB4BEFE_rgb .WithAlpha(0xFF)),       // Lavender: active border
		BorderFavorite(0xF9E2AF_rgb .WithAlpha(0xFF)),     // Yellow: favorite
		BorderDisabled(0x313244_rgb .WithAlpha(0xFF)),     // Surface0: disabled border

		Margin(1, 4),
		Border(1),

		icon(NoIcon)
	{}

	VideoBuffer const *Appearance::GetTexture()
	{
		return texture.get();
	}

	void Appearance::SetTexture(std::unique_ptr<VideoBuffer> texture)
	{
		this->texture = std::move(texture);
	}
}
