#include "simulation/ElementCommon.h"

static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_BRAS()
{
	Identifier = "DEFAULT_PT_BRAS";
	Name = "Brass";
	Colour = 0xC5AB6E_rgb;
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
	Hardness  = 0; // corrosion resistant like gold

	Weight = 100;

	HeatConduct = 210;
	Description = "Brass. Alloy of copper and zinc. Good conductor, corrosion resistant.";

	Properties = TYPE_SOLID | PROP_CONDUCTS | PROP_LIFE_DEC | PROP_HOT_GLOW;

	LowPressure            = IPL;
	LowPressureTransition  = NT;
	HighPressure           = IPH;
	HighPressureTransition = NT;
	LowTemperature            = ITL;
	LowTemperatureTransition  = NT;
	HighTemperature            = 1173.0f;
	HighTemperatureTransition  = PT_LAVA; //@ BRAS -> LAVA(BRAS)

	Graphics = &graphics;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	int rndstore = gfctx.rng.gen();
	*colr += (rndstore % 10) - 5;
	rndstore >>= 4;
	*colg += (rndstore % 8) - 4;
	return 0;
}
