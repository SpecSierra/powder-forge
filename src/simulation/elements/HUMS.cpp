#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_HUMS()
{
	Identifier = "DEFAULT_PT_HUMS";
	Name = "Humus";
	Colour = 0x2A1A0A_rgb;
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

	Flammable = 5;
	Explosive = 0;
	Meltable = 0;
	Hardness = 4;

	Weight = 75;

	HeatConduct = 65;
	Description = "Humus. Rich dark soil from decomposed matter. Seeds germinate instantly. Plants grow twice as fast in it.";

	Properties = TYPE_PART;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 700.0f;
	HighTemperatureTransition = PT_DIRT; //@ HUMS -> DIRT (organics burn off, leaves mineral soil)

	DefaultProperties.tmp = 0; // water content

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

		if (TYP(r) == PT_SEED && sim->rng.chance(1, 20))
		{
			//@ HUMS + SEED -> PLNT (instant germination in rich humus)
			sim->part_change_type(ID(r), x+rx, y+ry, PT_PLNT);
		}
		if (TYP(r) == PT_PLNT && sim->rng.chance(1, 200))
		{
			//@ PLNT spreads 2x faster in HUMS (rich soil)
			sim->part_change_type(i, x, y, PT_PLNT);
			return 1;
		}
		if ((TYP(r) == PT_WATR || TYP(r) == PT_DSTW) && parts[i].tmp < 4 && sim->rng.chance(1, 80))
		{
			//@ HUMS absorbs WATR (high water retention)
			sim->kill_part(ID(r));
			parts[i].tmp++;
		}
		// Oversaturated humus releases water and degrades to dirt
		if (parts[i].tmp >= 4 && sim->rng.chance(1, 500))
		{
			//@ HUMS (saturated) -> DIRT + WATR (nutrients leach away)
			sim->create_part(-1, x + sim->rng.between(-1,1), y + 1, PT_WATR);
			sim->part_change_type(i, x, y, PT_DIRT);
			return 1;
		}
	}
	return 0;
}
