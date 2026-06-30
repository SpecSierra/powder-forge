#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_FISH()
{
	Identifier = "DEFAULT_PT_FISH";
	Name = "Fish";
	Colour = 0xFF8020_rgb;
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;

	Advection = 0.5f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.98f;
	Loss = 0.88f;
	Collision = 0.0f;
	Gravity = 0.0f; // neutral buoyancy — swims at any depth
	Diffusion = 0.0f;
	HotAir = 0.000f * CFDS;
	Falldown = 2;

	Flammable = 5;
	Explosive = 0;
	Meltable = 0;
	Hardness = 2;

	Weight = 30; // same density as water

	HeatConduct = 30;
	Description = "Fish. Swims in water. Eats plankton. Dies in acid or when cooked. Avoid stinging jellyfish.";

	Properties = TYPE_LIQUID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 270.0f;
	LowTemperatureTransition = PT_ICEI; //@ FISH -> ICEI (frozen solid)
	HighTemperature = 360.0f;
	HighTemperatureTransition = PT_DUST; //@ FISH -> DUST (cooked/boiled)

	DefaultProperties.tmp = 0; // 0=swimming right, 1=swimming left
	DefaultProperties.life = 120; // health

	Update = &update;
	Graphics = &graphics;
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

		// Eat plankton and algae
		if ((TYP(r) == PT_PLNK || TYP(r) == PT_ALGN) && sim->rng.chance(1, 10))
		{
			//@ FISH eats PLNK/ALGN
			sim->kill_part(ID(r));
			if (parts[i].life < 120) parts[i].life++;
		}
		// Stung by jellyfish
		if (TYP(r) == PT_JLLY && sim->rng.chance(1, 20))
		{
			//@ FISH + JLLY -> dies (jellyfish sting)
			parts[i].life -= 10;
		}
		if (TYP(r) == PT_ACID && sim->rng.chance(1, 10))
		{
			//@ FISH + ACID -> dissolves
			sim->kill_part(i);
			return 1;
		}
	}

	if (!inWater)
	{
		// Suffocates out of water
		if (sim->rng.chance(1, 30))
			parts[i].life -= 3;
	}

	if (parts[i].life <= 0)
	{
		//@ FISH (life=0) -> DUST (dies)
		sim->part_change_type(i, x, y, PT_DUST);
		return 1;
	}

	if (!inWater) return 0;

	// Swimming: apply horizontal velocity in current direction
	float speed = 0.6f;
	parts[i].vx += (parts[i].tmp == 0) ? speed : -speed;

	// Subtle vertical bobbing
	if (sim->rng.chance(1, 15))
		parts[i].vy += sim->rng.between(-2, 2) * 0.12f;

	// Water drag
	parts[i].vx *= 0.78f;
	parts[i].vy *= 0.82f;

	// Randomly reverse direction (obstacle avoidance / natural behaviour)
	if (sim->rng.chance(1, 80))
		parts[i].tmp = 1 - parts[i].tmp;

	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	// Orange-gold shimmer, direction hint via slight red/yellow shift
	*colr = 0xFF;
	*colg = 0x80 + (int)(cpart->vx * 8.0f);
	if (*colg < 0x50) *colg = 0x50;
	if (*colg > 0xC0) *colg = 0xC0;
	*colb = 0x10;
	return 0;
}
