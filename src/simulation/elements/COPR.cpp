#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_COPR()
{
	Identifier = "DEFAULT_PT_COPR";
	Name = "Copper";
	Colour = 0xB87333_rgb;
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
	Hardness  = 1;

	Weight = 100;

	HeatConduct = 250;
	Description = "Copper. Dissolves in acid. Heat with zinc for brass, with tin for bronze.";

	Properties = TYPE_SOLID | PROP_CONDUCTS | PROP_LIFE_DEC | PROP_HOT_GLOW;

	LowPressure            = IPL;
	LowPressureTransition  = NT;
	HighPressure           = IPH;
	HighPressureTransition = NT;
	LowTemperature            = ITL;
	LowTemperatureTransition  = NT;
	HighTemperature            = 1358.0f;
	HighTemperatureTransition  = PT_LAVA; //@ COPR -> LAVA(COPR)

	Update   = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
{
	for (auto rx = -1; rx <= 1; rx++)
	for (auto ry = -1; ry <= 1; ry++)
	{
		if (!rx && !ry) continue;
		auto r = pmap[y+ry][x+rx];
		if (!r) continue;

		if (TYP(r) == PT_ZINC && parts[i].temp > 500.0f && sim->rng.chance(1, 100))
		{
			//@ COPR + ZINC (>500K) -> 2xBRAS (brass alloy forms)
			sim->part_change_type(i, x, y, PT_BRAS);
			sim->part_change_type(ID(r), x+rx, y+ry, PT_BRAS);
			return 1;
		}
		if (TYP(r) == PT_TINN && parts[i].temp > 400.0f && sim->rng.chance(1, 100))
		{
			//@ COPR + TINN (>400K) -> 2xBRNZ (bronze alloy forms)
			sim->part_change_type(i, x, y, PT_BRNZ);
			sim->part_change_type(ID(r), x+rx, y+ry, PT_BRNZ);
			return 1;
		}
		if (TYP(r) == PT_ACID && sim->rng.chance(1, 30))
		{
			//@ COPR + ACID -> CAUS (copper dissolves, forms corrosive copper compound)
			sim->part_change_type(i, x, y, PT_CAUS);
			return 1;
		}
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	// Subtle reddish shimmer to distinguish from IRON/METL
	int rndstore = gfctx.rng.gen();
	*colr += (rndstore % 8);
	rndstore >>= 3;
	*colg -= (rndstore % 6);
	return 0;
}
