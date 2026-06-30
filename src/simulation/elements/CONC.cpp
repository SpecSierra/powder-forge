#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_CONC()
{
	Identifier = "DEFAULT_PT_CONC";
	Name = "Concrete";
	Colour = 0xB4AEAA_rgb;
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
	Hardness = 0; // acid handled in update

	Weight = 100;

	HeatConduct = 80;
	Description = "Concrete. Mix chalk, sand, and water — then seed with one concrete particle to set. Crumbles to gravel under extreme pressure.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 50.0f;
	HighPressureTransition = PT_GRVL; //@ CONC -> GRVL (concrete crumbles under extreme pressure)
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 1500.0f;
	HighTemperatureTransition = PT_LAVA; //@ CONC -> LAVA (extreme heat)

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	bool hasCalk = false, hasSand = false, hasWatr = false;

	for (auto rx = -2; rx <= 2; rx++)
	for (auto ry = -2; ry <= 2; ry++)
	{
		if (!rx && !ry) continue;
		if (y+ry < 0 || y+ry >= YRES || x+rx < 0 || x+rx >= XRES) continue;
		auto r = pmap[y+ry][x+rx];
		if (!r) continue;

		if (TYP(r) == PT_CALK) hasCalk = true;
		if (TYP(r) == PT_SAND) hasSand = true;
		if (TYP(r) == PT_WATR || TYP(r) == PT_DSTW) hasWatr = true;

		if (TYP(r) == PT_ACID && sim->rng.chance(1, 150))
		{
			//@ CONC + ACID -> DUST (acid erosion)
			sim->part_change_type(i, x, y, PT_DUST);
			return 1;
		}
	}

	// Viral concrete setting: seed CONC spreads into adjacent CALK+SAND+WATR mix
	if (hasCalk && hasSand && hasWatr && sim->rng.chance(1, 80))
	{
		// Find an adjacent CALK or SAND particle to convert
		for (auto rx = -1; rx <= 1; rx++)
		for (auto ry = -1; ry <= 1; ry++)
		{
			if (!rx && !ry) continue;
			auto r = pmap[y+ry][x+rx];
			if (!r) continue;
			if (TYP(r) == PT_CALK || TYP(r) == PT_SAND)
			{
				//@ CONC seed + CALK/SAND + WATR -> CONC (concrete sets)
				sim->part_change_type(ID(r), x+rx, y+ry, PT_CONC);
				return 0;
			}
		}
	}

	return 0;
}
