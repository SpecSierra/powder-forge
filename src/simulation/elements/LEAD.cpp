#include "simulation/ElementCommon.h"

void Element::Element_LEAD()
{
	Identifier = "DEFAULT_PT_LEAD";
	Name = "Lead";
	Colour = 0x586276_rgb;
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag   = 0.00f * CFDS;
	AirLoss   = 0.90f;
	Loss      = 0.00f;
	Collision = 0.0f;
	Gravity   = 0.0f;
	Diffusion = 0.00f;
	HotAir    = 0.000f * CFDS;
	Falldown  = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable  = 1;
	Hardness  = 2;

	Weight = 100;

	HeatConduct = 50;
	Description = "Lead. Absorbs neutrons and radiation. Heat with tin for solder. Poor electrical conductor.";

	// No PROP_CONDUCTS — lead is a poor electrical conductor
	// PROP_NEUTABSORB — actively absorbs neutrons (radiation shielding)
	Properties = TYPE_SOLID | PROP_NEUTABSORB | PROP_HOT_GLOW;

	LowPressure            = IPL;
	LowPressureTransition  = NT;
	HighPressure           = IPH;
	HighPressureTransition = NT;
	LowTemperature            = ITL;
	LowTemperatureTransition  = NT;
	HighTemperature            = 600.0f;
	HighTemperatureTransition  = PT_LAVA; //@ LEAD -> LAVA(LEAD)
}
