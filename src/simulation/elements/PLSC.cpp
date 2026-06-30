#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_PLSC()
{
	Identifier = "DEFAULT_PT_PLSC";
	Name = "Plastic";
	Colour = 0xE0E8F8_rgb;
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

	Flammable = 10;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0; // acid resistant — handled in update (barely reacts)

	Weight = 100;

	HeatConduct = 10; // poor thermal conductor (insulator)
	Description = "Plastic. Petroleum polymer. Acid-resistant. Burns with toxic fumes (CL2). Forms when crude oil seed contacts hot crude. Poor heat conductor.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 500.0f;
	HighTemperatureTransition = PT_GOO; //@ PLSC -> GOO (melted plastic becomes gooey)

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

		// Viral formation: plastic seed polymerises hot crude oil
		if (TYP(r) == PT_CRUD && parts[ID(r)].temp > 700.0f && sim->rng.chance(1, 200))
		{
			//@ PLSC seed + CRUD (>700K) -> PLSC (cracking / polymerisation)
			sim->part_change_type(ID(r), x+rx, y+ry, PT_PLSC);
			sim->create_part(-1, x+rx + sim->rng.between(-1,1), y+ry - 1, PT_METH); // light gas byproduct
		}

		// Burning: slow ignition but produces toxic fumes
		if (TYP(r) == PT_FIRE && sim->rng.chance(1, 60))
		{
			//@ PLSC + FIRE -> CL2 + SMKE (burning plastic releases chlorine/toxic fumes)
			sim->create_part(-1, x + sim->rng.between(-1,1), y + sim->rng.between(-1,1), PT_CL2);
			sim->create_part(-1, x + sim->rng.between(-1,1), y - 1, PT_SMKE);
			parts[i].life--;
			if (parts[i].life <= 0)
			{
				sim->part_change_type(i, x, y, PT_DUST);
				return 1;
			}
		}
		if (TYP(r) == PT_LAVA && sim->rng.chance(1, 100))
		{
			//@ PLSC + LAVA -> GOO (plastic melts in extreme heat)
			sim->part_change_type(i, x, y, PT_GOO);
			return 1;
		}
	}
	return 0;
}
