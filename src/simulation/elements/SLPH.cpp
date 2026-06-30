#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_SLPH()
{
	Identifier = "DEFAULT_PT_SLPH";
	Name = "Sulfur";
	Colour = 0xE8D020_rgb;
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
	Enabled = 1;

	Advection = 0.4f;
	AirDrag = 0.04f * CFDS;
	AirLoss = 0.94f;
	Loss = 0.95f;
	Collision = -0.1f;
	Gravity = 0.3f;
	Diffusion = 0.00f;
	HotAir = 0.000f * CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 2;

	Weight = 87;

	HeatConduct = 100;
	Description = "Sulfur powder. Burns to produce SO2. Reacts with hydrogen to make H2S.";

	Properties = TYPE_PART;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 388.0f;
	HighTemperatureTransition = PT_LAVA; //@ SLPH -> LAVA (melts)

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

		if (TYP(r) == PT_FIRE && sim->rng.chance(1, 8))
		{
			//@ SLPH + FIRE -> SO2 (combustion)
			sim->part_change_type(i, x, y, PT_SO2);
			return 1;
		}
		if (TYP(r) == PT_H2 && parts[i].temp > 500.0f && sim->rng.chance(1, 100))
		{
			//@ SLPH + H2 (>500K) -> H2S (synthesis)
			sim->part_change_type(i, x, y, PT_H2S);
			sim->kill_part(ID(r));
			return 1;
		}
	}
	return 0;
}
