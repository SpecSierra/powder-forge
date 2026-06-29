#include "ToolButton.h"
#include "graphics/Graphics.h"
#include "graphics/Pixel.h"
#include "graphics/VideoBuffer.h"
#include "Favorite.h"
#include <SDL.h>
#include <algorithm>

ToolButton::ToolButton(ui::Point position, ui::Point size, String text, ByteString toolIdentifier, String toolTip):
	ui::Button(position, size, text, toolTip),
	toolIdentifier(toolIdentifier)
{
	SetSelectionState(-1);
	Appearance.BorderActive = ui::Colour(255, 0, 0);
	Appearance.BorderFavorite = ui::Colour(255, 255, 0);

	//don't use "..." on elements that have long names
	buttonDisplayText = ButtonText;
	Component::TextPosition(buttonDisplayText);
}

void ToolButton::OnMouseDown(int x, int y, unsigned int button)
{
	if (MouseDownInside)
	{
		isButtonDown = true;
	}
}

void ToolButton::OnMouseClick(int x, int y, unsigned int button)
{
	if(isButtonDown)
	{
		isButtonDown = false;
		if(button == SDL_BUTTON_LEFT)
			SetSelectionState(0);
		if(button == SDL_BUTTON_RIGHT)
			SetSelectionState(1);
		if(button == SDL_BUTTON_MIDDLE)
			SetSelectionState(2);
		DoAction();
	}
}

void ToolButton::OnMouseUp(int x, int y, unsigned int button)
{
	// mouse was unclicked, reset variables in case the unclick happened outside
	isButtonDown = false;
}

void ToolButton::Draw(const ui::Point& screenPos)
{
	Graphics * g = GetGraphics();
	auto rect = ClipRect;
	if (ClipRect.size.X && ClipRect.size.Y)
		g->SwapClipRect(rect);

	int totalColour = Appearance.BackgroundInactive.Blue + (3*Appearance.BackgroundInactive.Green) + (2*Appearance.BackgroundInactive.Red);

	// Hover background highlight
	if (isMouseInside && currentSelection == -1)
		g->BlendFilledRect(RectSized(screenPos + Vec2{ 1, 1 }, Size - Vec2{ 2, 2 }), 0x313244_rgb .WithAlpha(0xFF));

	if (Appearance.GetTexture())
	{
		auto *tex = Appearance.GetTexture();
		g->BlendImage(tex->Data(), 255, RectSized(screenPos + Vec2{ 2, 2 }, tex->Size()));
	}
	else
	{
		// Fill body with element color
		g->BlendFilledRect(RectSized(screenPos + Vec2{ 2, 2 }, Size - Vec2{ 4, 4 }), Appearance.BackgroundInactive);

		// Subtle inner highlight on top edge
		ui::Colour highlight(
			uint8_t(std::min(255, int(Appearance.BackgroundInactive.Red)   + 30)),
			uint8_t(std::min(255, int(Appearance.BackgroundInactive.Green) + 30)),
			uint8_t(std::min(255, int(Appearance.BackgroundInactive.Blue)  + 30)),
			uint8_t(80)
		);
		g->BlendLine(screenPos + Vec2{ 3, 2 }, screenPos + Vec2{ Size.X-4, 2 }, highlight);
	}

	ui::Colour borderCol = (isMouseInside && currentSelection == -1) ? Appearance.BorderActive : Appearance.BorderInactive;

	// Rounded-corner border for tool button
	if (Size.X >= 4 && Size.Y >= 4)
	{
		g->BlendLine(screenPos + Vec2{ 1, 0 }, screenPos + Vec2{ Size.X-2, 0 }, borderCol);
		g->BlendLine(screenPos + Vec2{ 1, Size.Y-1 }, screenPos + Vec2{ Size.X-2, Size.Y-1 }, borderCol);
		g->BlendLine(screenPos + Vec2{ 0, 1 }, screenPos + Vec2{ 0, Size.Y-2 }, borderCol);
		g->BlendLine(screenPos + Vec2{ Size.X-1, 1 }, screenPos + Vec2{ Size.X-1, Size.Y-2 }, borderCol);
		ui::Colour cornerDim(borderCol.Red, borderCol.Green, borderCol.Blue, uint8_t(borderCol.Alpha / 3));
		g->BlendPixel(screenPos + Vec2{ 0, 0 }, cornerDim);
		g->BlendPixel(screenPos + Vec2{ Size.X-1, 0 }, cornerDim);
		g->BlendPixel(screenPos + Vec2{ 0, Size.Y-1 }, cornerDim);
		g->BlendPixel(screenPos + Vec2{ Size.X-1, Size.Y-1 }, cornerDim);
	}
	else
	{
		g->BlendRect(RectSized(screenPos, Size), borderCol);
	}

	if (Favorite::Ref().IsFavorite(toolIdentifier))
		g->BlendText(screenPos, 0xE068, Appearance.BorderFavorite);

	if (totalColour < 544)
		g->BlendText(screenPos + textPosition, buttonDisplayText, 0xFFFFFF_rgb .WithAlpha(255));
	else
		g->BlendText(screenPos + textPosition, buttonDisplayText, 0x000000_rgb .WithAlpha(255));

	if (ClipRect.size.X && ClipRect.size.Y)
		g->SwapClipRect(rect);
}

void ToolButton::SetSelectionState(int state)
{
	currentSelection = state;
	switch(state)
	{
	case 0:
		Appearance.BorderInactive = ui::Colour(255, 0, 0);
		break;
	case 1:
		Appearance.BorderInactive = ui::Colour(0, 0, 255);
		break;
	case 2:
		Appearance.BorderInactive = ui::Colour(0, 255, 0);
		break;
	case 3:
		Appearance.BorderInactive = ui::Colour(0, 255, 255);
		break;
	default:
		Appearance.BorderInactive = ui::Colour(0, 0, 0);
		break;
	}
}

int ToolButton::GetSelectionState()
{
	return currentSelection;
}
