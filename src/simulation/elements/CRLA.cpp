#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_CRLA()
{
	Identifier = "DEFAULT_PT_CRLA";
	Name = "Coral";
	Colour = 0xFF7048_rgb;
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
	Hardness = 0; // CaCO3 shell, acid handled in update

	Weight = 100;

	HeatConduct = 30;
	Description = "Coral. Grows in salt water. Feeds on plankton. Bleaches above 308K. Produces oxygen.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 340.0f;
	HighTemperatureTransition = PT_CALK; //@ CRLA -> CALK (extreme heat kills coral, skeleton remains)

	Update = &update;
	Graphics = &graphics;
	Create = &create;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	// Assign a coral colour variety: 0=orange 1=pink 2=purple 3=yellow
	sim->parts[i].tmp = sim->rng.between(0, 3);
}

static int update(UPDATE_FUNC_ARGS)
{
	bool inSaltWater = false;
	bool inFreshWater = false;

	for (auto rx = -1; rx <= 1; rx++)
	for (auto ry = -1; ry <= 1; ry++)
	{
		if (!rx && !ry) continue;
		auto r = pmap[y+ry][x+rx];
		if (!r) continue;

		if (TYP(r) == PT_SLTW) inSaltWater = true;
		if (TYP(r) == PT_WATR || TYP(r) == PT_DSTW) inFreshWater = true;

		// Feed on plankton
		if (TYP(r) == PT_PLNK && sim->rng.chance(1, 15))
		{
			//@ CRLA eats PLNK (filter feeding)
			sim->kill_part(ID(r));
		}
		if (TYP(r) == PT_ACID && sim->rng.chance(1, 8))
		{
			//@ CRLA + ACID -> CAUS + CO2 (acid dissolves CaCO3 shell)
			sim->part_change_type(i, x, y, PT_CAUS);
			sim->create_part(-1, x + sim->rng.between(-1,1), y - 1, PT_CO2);
			return 1;
		}
	}

	// Coral bleaching: above 308K the symbiotic algae die
	if (parts[i].temp > 308.0f && sim->rng.chance(1, 150))
	{
		//@ CRLA (>308K) -> CALK (coral bleaching — skeleton remains)
		sim->part_change_type(i, x, y, PT_CALK);
		return 1;
	}

	// Dies in fresh water (osmotic stress)
	if (inFreshWater && !inSaltWater && sim->rng.chance(1, 200))
	{
		//@ CRLA + WATR (fresh) -> CALK (osmotic stress kills coral)
		sim->part_change_type(i, x, y, PT_CALK);
		return 1;
	}

	if (!inSaltWater) return 0;

	// Photosynthesis (from symbiotic zooxanthellae)
	if (sim->rng.chance(1, 1200))
	{
		//@ CRLA (in SLTW) -> O2 (symbiotic photosynthesis)
		sim->create_part(-1, x + sim->rng.between(-1,1), y - 1, PT_O2);
	}

	// Reef growth: spread to adjacent solid substrate in salt water
	int growChance = 1000;
	// Plankton nearby boosts growth rate
	for (auto rx = -2; rx <= 2; rx++)
	for (auto ry = -2; ry <= 2; ry++)
	{
		auto r = pmap[y+ry][x+rx];
		if (r && TYP(r) == PT_PLNK) { growChance = 400; break; }
	}

	if (sim->rng.chance(1, growChance))
	{
		int gx = x + sim->rng.between(-1, 1);
		int gy = y + sim->rng.between(-1, 1);
		if (gx != x || gy != y)
		{
			auto r = pmap[gy][gx];
			if (r && (TYP(r) == PT_STNE || TYP(r) == PT_SAND || TYP(r) == PT_GRVL
			       || TYP(r) == PT_ROCK || TYP(r) == PT_CALK || TYP(r) == PT_SLTW))
			{
				//@ CRLA grows into substrate (reef building)
				sim->part_change_type(ID(r), gx, gy, PT_CRLA);
				sim->parts[ID(r)].tmp = parts[i].tmp; // pass colour variety
			}
		}
	}

	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	// Four natural coral colours
	switch (cpart->tmp & 3)
	{
		case 0: *colr = 0xFF; *colg = 0x70; *colb = 0x48; break; // orange
		case 1: *colr = 0xFF; *colg = 0x58; *colb = 0xA8; break; // pink
		case 2: *colr = 0x90; *colg = 0x48; *colb = 0xD0; break; // purple
		default: *colr = 0xFF; *colg = 0xD0; *colb = 0x28; break; // yellow
	}
	// Subtle warm glow
	*firea = 30;
	*firer = *colr;
	*fireg = *colg / 2;
	*fireb = *colb / 2;
	*pixel_mode |= FIRE_ADD;
	return 0;
}
