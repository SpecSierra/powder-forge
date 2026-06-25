#include "gui/Style.h"

namespace style {
	// Catppuccin Mocha palette
	ui::Colour Colour::InformationTitle = ui::Colour(137, 180, 250); // Blue
	ui::Colour Colour::WarningTitle = ui::Colour(249, 226, 175);      // Yellow
	ui::Colour Colour::ErrorTitle = ui::Colour(243, 139, 168);        // Red/Pink

	ui::Colour Colour::ConfirmButton = ui::Colour(166, 227, 161);     // Green

	ui::Colour Colour::ActiveBorder = ui::Colour(137, 180, 250);      // Blue
	ui::Colour Colour::InactiveBorder = ui::Colour(69, 71, 90);       // Surface1

	ui::Colour Colour::ActiveBackground = ui::Colour(49, 50, 68);     // Surface0
	ui::Colour Colour::InactiveBackground = ui::Colour(30, 30, 46);   // Base
}
