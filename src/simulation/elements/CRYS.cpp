#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_CRYS()
{
	Identifier = "DEFAULT_PT_CRYS";
	Name = "Crystal";
	Colour = 0x80E0FF_rgb;
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
	Meltable = 0;
	Hardness = 0;

	Weight = 100;

	HeatConduct = 30;
	Description = "Salt crystal. Grows from evaporating salt water. Piezoelectric: generates sparks under pressure.";

	Properties = TYPE_SOLID | PROP_PHOTPASS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 20.0f;
	HighPressureTransition = PT_SAND; //@ CRYS -> SAND (shatters under extreme pressure)
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 1500.0f;
	HighTemperatureTransition = PT_SLTW; //@ CRYS -> SLTW (melts to salt water)

	Update = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
{
	float pv = sim->pv[y/CELL][x/CELL];

	// Piezoelectric effect: high pressure generates electricity
	if (pv > 5.0f && sim->rng.chance(1, 30))
	{
		auto &sd = SimulationData::CRef();
		for (auto rx = -1; rx <= 1; rx++)
		for (auto ry = -1; ry <= 1; ry++)
		{
			if (!rx && !ry) continue;
			auto r = pmap[y+ry][x+rx];
			if (!r) continue;
			if (sd.elements[TYP(r)].Properties & PROP_CONDUCTS)
			{
				//@ CRYS (under pressure) -> SPRK on adjacent conductors (piezoelectric)
				sim->part_change_type(ID(r), x+rx, y+ry, PT_SPRK);
				parts[ID(r)].ctype = TYP(r);
				parts[ID(r)].life = 4;
			}
		}
	}

	for (auto rx = -1; rx <= 1; rx++)
	for (auto ry = -1; ry <= 1; ry++)
	{
		if (!rx && !ry) continue;
		auto r = pmap[y+ry][x+rx];
		if (!r) continue;

		// Crystallization: grow from hot salt water
		if (TYP(r) == PT_SLTW && parts[ID(r)].temp > 360.0f && sim->rng.chance(1, 300))
		{
			//@ CRYS + SLTW (>360K) -> CRYS grows (evaporative crystallization)
			sim->part_change_type(ID(r), x+rx, y+ry, PT_CRYS);
		}
		if (TYP(r) == PT_ACID && sim->rng.chance(1, 10))
		{
			//@ CRYS + ACID -> dissolves
			sim->kill_part(i);
			return 1;
		}
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	// Sparkle/shimmer: vary by position for a static crystal glint
	*firea = 60;
	*firer = 0x80;
	*fireg = 0xE0;
	*fireb = 0xFF;
	*pixel_mode |= FIRE_ADD | PMODE_SPARK;
	return 0;
}
