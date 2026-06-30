#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_ASPH()
{
	Identifier = "DEFAULT_PT_ASPH";
	Name = "Asphalt";
	Colour = 0x100E0C_rgb;
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

	Flammable = 3;
	Explosive = 0;
	Meltable = 0;
	Hardness = 5;

	Weight = 100;

	HeatConduct = 60;
	Description = "Asphalt. Bitumen mixed with aggregate. Seed near crude oil or tar with gravel to form. Hard road surface. Burns slowly.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 600.0f;
	HighTemperatureTransition = PT_LAVA; //@ ASPH -> LAVA (melts and flows at very high temp)

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

		// CRUD/TARZ binding with aggregate → more asphalt (viral spread)
		bool isBitumen = (TYP(r) == PT_CRUD || TYP(r) == PT_TARZ);
		if (isBitumen && sim->rng.chance(1, 100))
		{
			// Look for aggregate nearby
			for (auto rx2 = -2; rx2 <= 2; rx2++)
			for (auto ry2 = -2; ry2 <= 2; ry2++)
			{
				if (y+ry2 < 0 || y+ry2 >= YRES || x+rx2 < 0 || x+rx2 >= XRES) continue;
				auto r2 = pmap[y+ry2][x+rx2];
				if (r2 && (TYP(r2) == PT_GRVL || TYP(r2) == PT_STNE || TYP(r2) == PT_SAND))
				{
					//@ ASPH seed + CRUD/TARZ + GRVL/STNE -> ASPH (asphalt paving)
					sim->part_change_type(ID(r), x+rx, y+ry, PT_ASPH);
					break;
				}
			}
		}

		if (TYP(r) == PT_ACID && sim->rng.chance(1, 250))
		{
			//@ ASPH + ACID -> TARZ (acid strips binding, leaves raw tar)
			sim->part_change_type(i, x, y, PT_TARZ);
			return 1;
		}
		if (TYP(r) == PT_FIRE && sim->rng.chance(1, 200))
		{
			//@ ASPH + FIRE -> slowly starts burning (produces SMKE)
			sim->create_part(-1, x + sim->rng.between(-1,1), y - 1, PT_SMKE);
			parts[i].life++;
			if (parts[i].life > 30)
			{
				sim->part_change_type(i, x, y, PT_TARZ); // burns back to raw tar
				return 1;
			}
		}
	}
	return 0;
}
