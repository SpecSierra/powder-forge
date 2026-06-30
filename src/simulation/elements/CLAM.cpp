#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_CLAM()
{
	Identifier = "DEFAULT_PT_CLAM";
	Name = "Clam";
	Colour = 0xA89880_rgb;
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
	Hardness = 0; // thick shell — acid handled in update

	Weight = 100;

	HeatConduct = 25;
	Description = "Clam. Filter-feeds on plankton. Hard shell resists acid. Dies without water, leaving a calcium shell.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 25.0f;
	HighPressureTransition = PT_CALK; //@ CLAM -> CALK (crushed shell)
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 370.0f;
	HighTemperatureTransition = PT_CALK; //@ CLAM -> CALK (cooked, shell remains)

	DefaultProperties.life = 200; // health

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	bool inWater = false;

	for (auto rx = -2; rx <= 2; rx++)
	for (auto ry = -2; ry <= 2; ry++)
	{
		if (!rx && !ry) continue;
		auto r = pmap[y+ry][x+rx];
		if (!r) continue;

		if (TYP(r) == PT_WATR || TYP(r) == PT_SLTW || TYP(r) == PT_DSTW)
			inWater = true;

		// Filter feed: eat nearby plankton
		if (TYP(r) == PT_PLNK && sim->rng.chance(1, 15))
		{
			//@ CLAM eats PLNK (filter feeding)
			sim->kill_part(ID(r));
			if (parts[i].life < 200)
				parts[i].life += 2; // eating restores health
		}

		// ALGN is also edible
		if (TYP(r) == PT_ALGN && sim->rng.chance(1, 30))
		{
			//@ CLAM eats ALGN
			sim->kill_part(ID(r));
			if (parts[i].life < 200)
				parts[i].life++;
		}

		if (TYP(r) == PT_ACID && sim->rng.chance(1, 40))
		{
			//@ CLAM + ACID -> CALK (acid slowly dissolves shell)
			sim->part_change_type(i, x, y, PT_CALK);
			return 1;
		}
	}

	if (!inWater)
	{
		// Slowly dies without water
		if (sim->rng.chance(1, 100))
		{
			parts[i].life -= 3;
		}
	}

	// Death from starvation/exposure
	if (parts[i].life <= 0)
	{
		//@ CLAM (life=0) -> CALK (dies, shell remains)
		sim->part_change_type(i, x, y, PT_CALK);
		return 1;
	}

	// Slow natural life drain (aging)
	if (sim->rng.chance(1, 1200))
		parts[i].life--;

	return 0;
}
