#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_KELP()
{
	Identifier = "DEFAULT_PT_KELP";
	Name = "Kelp";
	Colour = 0x1A6010_rgb;
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f * CFDS;
	Falldown = 0;

	Flammable = 15;
	Explosive = 0;
	Meltable = 0;
	Hardness = 5;

	Weight = 100;

	HeatConduct = 40;
	Description = "Kelp. Grows upward in water. Produces oxygen. Dies and dries out when exposed to air.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 370.0f;
	HighTemperatureTransition = PT_FIRE; //@ KELP -> FIRE (dries and burns)

	Update = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
{
	bool hasWater = false;

	for (auto rx = -1; rx <= 1; rx++)
	for (auto ry = -1; ry <= 1; ry++)
	{
		if (!rx && !ry) continue;
		auto r = pmap[y+ry][x+rx];
		if (!r) continue;

		if (TYP(r) == PT_WATR || TYP(r) == PT_SLTW || TYP(r) == PT_DSTW)
			hasWater = true;

		if (TYP(r) == PT_ACID && sim->rng.chance(1, 15))
		{
			//@ KELP + ACID -> dissolves
			sim->kill_part(i);
			return 1;
		}
	}

	if (!hasWater)
	{
		// Drying out — slowly dies without water
		if (sim->rng.chance(1, 120))
		{
			//@ KELP (no water) -> DUST (desiccation)
			sim->part_change_type(i, x, y, PT_DUST);
			return 1;
		}
		return 0;
	}

	// Grow upward: convert the water cell directly above into kelp
	if (y > 1 && sim->rng.chance(1, 100))
	{
		auto above = pmap[y-1][x];
		if (above && (TYP(above) == PT_WATR || TYP(above) == PT_SLTW || TYP(above) == PT_DSTW))
		{
			//@ KELP grows upward into water
			sim->part_change_type(ID(above), x, y-1, PT_KELP);
		}
	}

	// Photosynthesis: slowly produce O2
	if (sim->rng.chance(1, 800))
	{
		//@ KELP (in water) -> O2 (photosynthesis)
		sim->create_part(-1, x + sim->rng.between(-1,1), y - 1, PT_O2);
	}

	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	// Slight wave shimmer based on position
	int wave = (int)(cpart->temp - 273.15f) & 0x7;
	*colg = 0x60 + wave * 4;
	*colr = 0x1A + wave;
	return 0;
}
