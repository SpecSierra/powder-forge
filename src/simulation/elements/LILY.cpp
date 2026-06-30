#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_LILY()
{
	Identifier = "DEFAULT_PT_LILY";
	Name = "Water Lily";
	Colour = 0x1A7010_rgb;
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

	Flammable = 12;
	Explosive = 0;
	Meltable = 0;
	Hardness = 5;

	Weight = 100;

	HeatConduct = 40;
	Description = "Water Lily. Floats on water surface, spreading sideways. Produces oxygen. Some cells bloom into flowers.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 380.0f;
	HighTemperatureTransition = PT_FIRE; //@ LILY -> FIRE (burns)

	DefaultProperties.tmp = 0; // 0=pad, 1=flower

	Update = &update;
	Graphics = &graphics;
	Create = &create;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	// 15% chance of spawning as a flower instead of a pad
	sim->parts[i].tmp = (sim->rng.chance(15, 100)) ? 1 : 0;
}

static int update(UPDATE_FUNC_ARGS)
{
	bool hasWaterAdjacent = false;

	for (auto rx = -1; rx <= 1; rx++)
	for (auto ry = -1; ry <= 1; ry++)
	{
		if (!rx && !ry) continue;
		auto r = pmap[y+ry][x+rx];
		if (!r) continue;

		if (TYP(r) == PT_WATR || TYP(r) == PT_SLTW || TYP(r) == PT_DSTW)
			hasWaterAdjacent = true;
		if (TYP(r) == PT_ACID && sim->rng.chance(1, 20))
		{
			//@ LILY + ACID -> dissolves
			sim->kill_part(i);
			return 1;
		}
	}

	// Dies if fully disconnected from water (stranded on dry land)
	if (!hasWaterAdjacent && sim->rng.chance(1, 200))
	{
		//@ LILY (no water) -> DUST (desiccates)
		sim->part_change_type(i, x, y, PT_DUST);
		return 1;
	}

	if (!hasWaterAdjacent) return 0;

	// Photosynthesis
	if (sim->rng.chance(1, 600))
	{
		//@ LILY -> O2 (photosynthesis)
		sim->create_part(-1, x + sim->rng.between(-1,1), y - 1, PT_O2);
	}

	// Pad growth: spread sideways along water surface
	if (parts[i].tmp == 0 && sim->rng.chance(1, 150))
	{
		int dir = sim->rng.chance(1, 2) ? -1 : 1;
		auto r = pmap[y][x + dir];
		if (r && (TYP(r) == PT_WATR || TYP(r) == PT_SLTW || TYP(r) == PT_DSTW))
		{
			//@ LILY pad spreads sideways along water surface
			sim->part_change_type(ID(r), x + dir, y, PT_LILY);
			// Flowers only bloom occasionally at the growing edge
			sim->parts[ID(r)].tmp = sim->rng.chance(1, 8) ? 1 : 0;
		}
	}

	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	if (cpart->tmp == 1)
	{
		// Flower: white with pink centre
		*colr = 0xFF;
		*colg = 0xE0;
		*colb = 0xF0;
		*firea = 40;
		*firer = 0xFF;
		*fireg = 0xA0;
		*fireb = 0xC0;
		*pixel_mode |= FIRE_ADD;
	}
	else
	{
		// Lily pad: dark green with slight variation
		*colr = 0x1A;
		*colg = 0x70 + (int)(cpart->temp * 0.05f) % 16;
		*colb = 0x10;
	}
	return 0;
}
