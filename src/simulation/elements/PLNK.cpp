#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_PLNK()
{
	Identifier = "DEFAULT_PT_PLNK";
	Name = "Plankton";
	Colour = 0x40C878_rgb;
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;

	Advection = 0.6f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.98f;
	Loss = 0.96f;
	Collision = 0.0f;
	Gravity = 0.04f;
	Diffusion = 0.4f; // drifts in water currents
	HotAir = 0.000f * CFDS;
	Falldown = 2;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = 29; // slightly lighter than water — stays near surface

	HeatConduct = 35;
	Description = "Plankton. Microscopic ocean life. Photosynthesises CO2 into oxygen. Feeds coral and clams.";

	Properties = TYPE_LIQUID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 270.0f;
	LowTemperatureTransition = PT_ICEI; //@ PLNK -> ICEI (freezes)
	HighTemperature = 330.0f;
	HighTemperatureTransition = PT_DUST; //@ PLNK -> DUST (dies in warm water)

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	bool inWater = false;

	for (auto rx = -1; rx <= 1; rx++)
	for (auto ry = -1; ry <= 1; ry++)
	{
		if (!rx && !ry) continue;
		auto r = pmap[y+ry][x+rx];
		if (!r) continue;

		if (TYP(r) == PT_WATR || TYP(r) == PT_SLTW || TYP(r) == PT_DSTW)
			inWater = true;

		if (TYP(r) == PT_ACID && sim->rng.chance(1, 15))
		{
			//@ PLNK + ACID -> dissolves
			sim->kill_part(i);
			return 1;
		}
	}

	if (!inWater)
	{
		// Dies quickly out of water
		if (sim->rng.chance(1, 30))
		{
			sim->part_change_type(i, x, y, PT_DUST);
			return 1;
		}
		return 0;
	}

	// Photosynthesis: consume CO2, produce O2, slowly bloom
	for (auto rx = -2; rx <= 2; rx++)
	for (auto ry = -2; ry <= 2; ry++)
	{
		if (!rx && !ry) continue;
		auto r = pmap[y+ry][x+rx];
		if (!r) continue;

		if (TYP(r) == PT_CO2
			&& parts[i].temp > 273.0f && parts[i].temp < 320.0f
			&& sim->rng.chance(1, 60))
		{
			//@ PLNK + CO2 (273-320K) -> O2 (photosynthesis)
			sim->part_change_type(ID(r), x+rx, y+ry, PT_O2);

			// Bloom: small chance to reproduce into adjacent water
			if (sim->rng.chance(1, 8))
			{
				int bx = x + sim->rng.between(-2, 2);
				int by = y + sim->rng.between(-2, 2);
				auto br = pmap[by][bx];
				if (br && (TYP(br) == PT_WATR || TYP(br) == PT_SLTW))
				{
					//@ PLNK blooms (grows into adjacent water when CO2 rich)
					sim->part_change_type(ID(br), bx, by, PT_PLNK);
				}
			}
		}
	}

	return 0;
}
