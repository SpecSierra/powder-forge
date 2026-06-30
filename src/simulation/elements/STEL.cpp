#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_STEL()
{
	Identifier = "DEFAULT_PT_STEL";
	Name = "Steel";
	Colour = 0xA0A8B8_rgb;
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

	Flammable = 0;
	Explosive = 0;
	Meltable = 1;
	Hardness = 0; // handled in update — acid resistance much higher than iron

	Weight = 100;

	HeatConduct = 230;
	Description = "Steel. Iron-carbon alloy. Forms when molten iron contacts coal or charcoal at 1500K+. Stronger than iron, conducts electricity.";

	Properties = TYPE_SOLID | PROP_CONDUCTS | PROP_HOT_GLOW;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 1811.0f;
	HighTemperatureTransition = PT_LAVA; //@ STEL -> LAVA (steel melts)

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	for (auto rx = -1; rx <= 1; rx++)
	for (auto ry = -1; ry <= 1; ry++)
	{
		if (!rx && !ry) continue;
		auto r = pmap[y+ry][x+rx];
		if (!r) continue;

		if (TYP(r) == PT_ACID && sim->rng.chance(1, 80))
		{
			//@ STEL + ACID -> IRON (acid strips carbon, decarburises)
			sim->part_change_type(i, x, y, PT_IRON);
			return 1;
		}
		if (TYP(r) == PT_WATR && sim->rng.chance(1, 4000))
		{
			//@ STEL + WATR -> RUST (slow surface rusting)
			sim->part_change_type(i, x, y, PT_RUST);
			return 1;
		}
	}
	return 0;
}
