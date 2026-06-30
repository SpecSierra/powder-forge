#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_CALK()
{
	Identifier = "DEFAULT_PT_CALK";
	Name = "Chalk";
	Colour = 0xF0EFE0_rgb;
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
	Hardness = 0; // handle acid reaction manually in update

	Weight = 85;

	HeatConduct = 50;
	Description = "Chalk (CaCO3). Fizzes with acid releasing CO2. High pressure forms limestone. Thermally decomposes at extreme heat.";

	Properties = TYPE_PART;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 15.0f;
	HighPressureTransition = PT_STNE; //@ CALK -> STNE (pressure-compressed into limestone)
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 1100.0f;
	HighTemperatureTransition = PT_LAVA; //@ CALK -> LAVA (thermal decomposition)

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

		if (TYP(r) == PT_ACID && sim->rng.chance(1, 10))
		{
			//@ CALK + ACID -> CO2 + WATR (CaCO3 + acid fizzing reaction)
			sim->part_change_type(i, x, y, PT_CO2);
			sim->create_part(-1, x + sim->rng.between(-1,1), y - 1, PT_WATR);
			// Consume some acid too
			if (sim->rng.chance(1, 3))
				sim->kill_part(ID(r));
			return 1;
		}
		if (TYP(r) == PT_WATR && sim->rng.chance(1, 1000))
		{
			//@ CALK + WATR -> slowly dissolves (WATR becomes SLTW — lime water)
			sim->part_change_type(ID(r), x+rx, y+ry, PT_SLTW);
		}
		if ((TYP(r) == PT_CAUS || TYP(r) == PT_SO2) && sim->rng.chance(1, 20))
		{
			//@ CALK + CAUS/SO2 -> neutralized (absorbed by chalk)
			sim->kill_part(ID(r));
		}
	}
	return 0;
}
