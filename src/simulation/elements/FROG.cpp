#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_FROG()
{
	Identifier = "DEFAULT_PT_FROG";
	Name = "Frog";
	Colour = 0x30B820_rgb;
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;

	Advection = 0.4f;
	AirDrag = 0.02f * CFDS;
	AirLoss = 0.97f;
	Loss = 0.85f;
	Collision = 0.0f;
	Gravity = 0.08f; // sinks gently when not jumping
	Diffusion = 0.0f;
	HotAir = 0.000f * CFDS;
	Falldown = 2;

	Flammable = 5;
	Explosive = 0;
	Meltable = 0;
	Hardness = 2;

	Weight = 32; // slightly heavier than water — rests on bottom

	HeatConduct = 30;
	Description = "Frog. Jumps in fresh water. Dies in salt water. Eats plankton. Sensitive to pollution.";

	Properties = TYPE_LIQUID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 270.0f;
	LowTemperatureTransition = PT_ICEI; //@ FROG -> ICEI (frozen)
	HighTemperature = 330.0f;
	HighTemperatureTransition = PT_DUST; //@ FROG -> DUST (overheated)

	DefaultProperties.life = 100;
	DefaultProperties.tmp = 0; // jump cooldown

	Update = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
{
	bool inFreshWater = false;
	bool inSaltWater = false;

	for (auto rx = -1; rx <= 1; rx++)
	for (auto ry = -1; ry <= 1; ry++)
	{
		if (!rx && !ry) continue;
		auto r = pmap[y+ry][x+rx];
		if (!r) continue;

		if (TYP(r) == PT_WATR || TYP(r) == PT_DSTW) inFreshWater = true;
		if (TYP(r) == PT_SLTW) inSaltWater = true;

		// Eat plankton
		if ((TYP(r) == PT_PLNK || TYP(r) == PT_ALGN) && sim->rng.chance(1, 20))
		{
			//@ FROG eats PLNK/ALGN
			sim->kill_part(ID(r));
			if (parts[i].life < 100) parts[i].life += 2;
		}
		// Dies in acid
		if (TYP(r) == PT_ACID && sim->rng.chance(1, 10))
		{
			sim->kill_part(i);
			return 1;
		}
	}

	// Salt water is toxic to frogs (osmotic stress)
	if (inSaltWater && sim->rng.chance(1, 60))
	{
		//@ FROG + SLTW -> dies (salt water kills frogs)
		parts[i].life -= 5;
	}

	// SO2 / CAUS in water kills frogs (pollution sensitivity)
	for (auto rx = -2; rx <= 2; rx++)
	for (auto ry = -2; ry <= 2; ry++)
	{
		auto r = pmap[y+ry][x+rx];
		if (r && (TYP(r) == PT_SO2 || TYP(r) == PT_CL2 || TYP(r) == PT_CAUS) && sim->rng.chance(1, 40))
		{
			//@ FROG + pollution (SO2/CL2/CAUS) -> harms frog
			parts[i].life -= 3;
		}
	}

	if (parts[i].life <= 0)
	{
		//@ FROG (life=0) -> DUST
		sim->part_change_type(i, x, y, PT_DUST);
		return 1;
	}

	// Slow natural aging
	if (sim->rng.chance(1, 800))
		parts[i].life--;

	if (!inFreshWater && !inSaltWater)
	{
		// Can survive in air briefly, but slowly loses health
		if (sim->rng.chance(1, 100))
			parts[i].life--;
	}

	// Jumping: periodic powerful upward leap
	if (parts[i].tmp > 0)
		parts[i].tmp--; // countdown between jumps

	if (parts[i].tmp == 0 && sim->rng.chance(1, 60))
	{
		//@ FROG jumps (leaping behavior)
		parts[i].vy -= 2.5f;
		parts[i].vx += sim->rng.between(-3, 3) * 0.25f;
		parts[i].tmp = 50; // wait 50 frames before next jump
	}

	// Dampen horizontal drift
	parts[i].vx *= 0.85f;

	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	// Bright green, slight spot pattern via position
	*colr = 0x28 + ((int)(cpart->temp) & 0x8);
	*colg = 0xB8;
	*colb = 0x18 + ((int)(cpart->temp) & 0x8);
	return 0;
}
