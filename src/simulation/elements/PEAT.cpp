#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_PEAT()
{
	Identifier = "DEFAULT_PT_PEAT";
	Name = "Peat";
	Colour = 0x3D2B1F_rgb;
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

	Flammable = 8;
	Explosive = 0;
	Meltable = 0;
	Hardness = 10;

	Weight = 100;

	HeatConduct = 80;
	Description = "Peat. Compressed organic matter. Absorbs water. Burns slowly. Under pressure becomes coal.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 650.0f;
	HighTemperatureTransition = PT_FIRE; //@ PEAT -> FIRE (ignites, lower temp than coal)

	DefaultProperties.tmp = 0; // water content (0-5)

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	// Geological compression: under high pressure, peat slowly becomes coal
	if (sim->pv[y/CELL][x/CELL] > 20.0f && sim->rng.chance(1, 4000))
	{
		//@ PEAT (high pressure) -> COAL (geological compression)
		sim->part_change_type(i, x, y, PT_COAL);
		return 1;
	}

	for (auto rx = -1; rx <= 1; rx++)
	for (auto ry = -1; ry <= 1; ry++)
	{
		if (!rx && !ry) continue;
		auto r = pmap[y+ry][x+rx];
		if (!r) continue;

		// Absorb water
		if ((TYP(r) == PT_WATR || TYP(r) == PT_DSTW) && parts[i].tmp < 5 && sim->rng.chance(1, 80))
		{
			//@ PEAT absorbs WATR (water retention)
			sim->kill_part(ID(r));
			parts[i].tmp++;
		}

		// Water content slows ignition: if wet, release steam instead of burning
		if (TYP(r) == PT_FIRE && parts[i].tmp > 0 && sim->rng.chance(1, 20))
		{
			//@ PEAT (wet) + FIRE -> WTRV (steam released before burning)
			sim->create_part(-1, x + sim->rng.between(-1,1), y - 1, PT_WTRV);
			parts[i].tmp--;
		}
	}
	return 0;
}
