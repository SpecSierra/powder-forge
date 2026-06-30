#include "simulation/ElementCommon.h"

void Element::Element_SOLD()
{
	Identifier = "DEFAULT_PT_SOLD";
	Name = "Solder";
	Colour = 0x8C9EB4_rgb;
	MenuVisible = 1;
	MenuSection = SC_ELEC;
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

	HeatConduct = 130;
	Description = "Solder. Tin-lead alloy. Very low melting point (456K/183C). Melts in fire — acts as circuit fuse.";

	Properties = TYPE_SOLID | PROP_CONDUCTS | PROP_LIFE_DEC | PROP_HOT_GLOW;

	LowPressure            = IPL;
	LowPressureTransition  = NT;
	HighPressure           = IPH;
	HighPressureTransition = NT;
	LowTemperature            = ITL;
	LowTemperatureTransition  = NT;
	HighTemperature            = 456.0f;
	HighTemperatureTransition  = PT_LAVA; //@ SOLD -> LAVA(SOLD)
}
