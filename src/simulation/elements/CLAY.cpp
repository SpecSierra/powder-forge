#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_CLAY()
{
	Identifier = "DEFAULT_PT_CLAY";
	Name = "Clay";
	Colour = 0x8B6914_rgb;
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
	Enabled = 1;

	Advection = 0.2f;
	AirDrag = 0.02f * CFDS;
	AirLoss = 0.95f;
	Loss = 0.65f;
	Collision = 0.0f;
	Gravity = 0.10f;
	Diffusion = 0.00f;
	HotAir = 0.000f * CFDS;
	Falldown = 2;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 20;

	Weight = 38;

	HeatConduct = 50;
	Description = "Wet clay. Fire-hardened into ceramic. Freezes solid when cold. Forms from water and stone.";

	Properties = TYPE_LIQUID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 250.0f;
	LowTemperatureTransition = PT_STNE; //@ CLAY -> STNE (freezes/hardens)
	HighTemperature = 900.0f;
	HighTemperatureTransition = PT_CRMC; //@ CLAY -> CRMC (fired ceramic)

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

		// WATR + STNE/SAND -> slowly produce CLAY (weathering)
		if (TYP(r) == PT_WATR && sim->rng.chance(1, 5000))
		{
			auto rr = pmap[y+ry*2][x+rx*2];
			if (rr && (TYP(rr) == PT_STNE || TYP(rr) == PT_SAND))
			{
				//@ CLAY grows near WATR+STNE (weathering)
				sim->part_change_type(ID(rr), x+rx*2, y+ry*2, PT_CLAY);
				sim->kill_part(ID(r));
			}
		}
		if (TYP(r) == PT_ACID && sim->rng.chance(1, 30))
		{
			//@ CLAY + ACID -> DUST
			sim->part_change_type(i, x, y, PT_DUST);
			return 1;
		}
	}
	return 0;
}
