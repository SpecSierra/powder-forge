#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_PAPE()
{
	Identifier = "DEFAULT_PT_PAPE";
	Name = "Paper";
	Colour = 0xFEFEF4_rgb;
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

	Flammable = 50;
	Explosive = 0;
	Meltable = 0;
	Hardness = 8;

	Weight = 30; // paper is very light

	HeatConduct = 120;
	Description = "Paper. Very flammable — ignites at 503K (Fahrenheit 451). Absorbs water, turning back to pulp. Made from sawdust and water.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 503.0f; // Fahrenheit 451 — the auto-ignition point of paper
	HighTemperatureTransition = PT_FIRE; //@ PAPE -> FIRE (Fahrenheit 451)

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

		// Water absorption: paper gets wet and falls apart into pulp
		if ((TYP(r) == PT_WATR || TYP(r) == PT_DSTW || TYP(r) == PT_SLTW) && sim->rng.chance(1, 20))
		{
			//@ PAPE + WATR -> SAWD (paper absorbs water and dissolves into pulp)
			sim->part_change_type(i, x, y, PT_SAWD);
			sim->kill_part(ID(r)); // water absorbed
			return 1;
		}

		// Viral papermaking: paper seed + sawdust + water → paper (drying process)
		if (TYP(r) == PT_SAWD && parts[i].temp > 320.0f && sim->rng.chance(1, 200))
		{
			// Check for water nearby to confirm pulp conditions
			bool hasWater = false;
			for (auto rx2 = -2; rx2 <= 2 && !hasWater; rx2++)
			for (auto ry2 = -2; ry2 <= 2 && !hasWater; ry2++) {
				auto r2 = pmap[y+ry2][x+rx2];
				if (r2 && (TYP(r2) == PT_WATR || TYP(r2) == PT_DSTW))
					hasWater = true;
			}
			if (hasWater)
			{
				//@ PAPE seed + SAWD + WATR (>320K) -> PAPE (paper forming from pulp)
				sim->part_change_type(ID(r), x+rx, y+ry, PT_PAPE);
			}
		}

		// ACID dissolves paper
		if (TYP(r) == PT_ACID && sim->rng.chance(1, 15))
		{
			//@ PAPE + ACID -> DUST
			sim->part_change_type(i, x, y, PT_DUST);
			return 1;
		}
	}
	return 0;
}
