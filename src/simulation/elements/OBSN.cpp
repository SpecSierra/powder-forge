#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_OBSN()
{
	Identifier = "DEFAULT_PT_OBSN";
	Name = "Obsidian";
	Colour = 0x0A0818_rgb;
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag   = 0.00f * CFDS;
	AirLoss   = 0.90f;
	Loss      = 0.00f;
	Collision = 0.0f;
	Gravity   = 0.0f;
	Diffusion = 0.00f;
	HotAir    = 0.000f * CFDS;
	Falldown  = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable  = 0;
	Hardness  = 0; // acid-proof volcanic glass

	Weight = 100;

	HeatConduct = 100;
	Description = "Obsidian. Volcanic glass formed when lava meets water. Shatters under pressure. Acid-proof.";

	Properties = TYPE_SOLID | PROP_HOT_GLOW;

	LowPressure            = IPL;
	LowPressureTransition  = NT;
	HighPressure           = IPH;
	HighPressureTransition = NT;
	LowTemperature            = ITL;
	LowTemperatureTransition  = NT;
	HighTemperature            = 2200.0f;
	HighTemperatureTransition  = PT_LAVA; //@ OBSN -> LAVA (melts back into lava)

	Update = &update;
	Create = &create;
}

static int update(UPDATE_FUNC_ARGS)
{
	// Shatter if pressure changes rapidly (volcanic glass is brittle)
	auto press = (int)(sim->pv[y/CELL][x/CELL] * 64);
	auto diff = press - parts[i].tmp;
	if (diff > 20 || diff < -20)
	{
		//@ OBSN -> BGLA (shatters under sudden pressure change)
		sim->part_change_type(i, x, y, PT_BGLA);
		return 1;
	}
	parts[i].tmp = press;
	return 0;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	// Record current pressure so the diff check starts from baseline
	sim->parts[i].tmp = (int)(sim->pv[y/CELL][x/CELL] * 64);
}
