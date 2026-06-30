#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_JLLY()
{
	Identifier = "DEFAULT_PT_JLLY";
	Name = "Jellyfish";
	Colour = 0x90D8FF_rgb;
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;

	Advection = 0.5f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.98f;
	Loss = 0.92f;
	Collision = 0.0f;
	Gravity = -0.03f; // slowly floats upward
	Diffusion = 0.2f;
	HotAir = 0.000f * CFDS;
	Falldown = 2;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 2;

	Weight = 27; // lighter than water — rises

	HeatConduct = 30;
	Description = "Jellyfish. Floats upward in water with pulsing motion. Deadly sting. Evaporates in air.";

	Properties = TYPE_LIQUID | PROP_DEADLY;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 270.0f;
	LowTemperatureTransition = PT_ICEI; //@ JLLY -> ICEI (freezes solid)
	HighTemperature = 315.0f;
	HighTemperatureTransition = PT_WTRV; //@ JLLY -> WTRV (dies/evaporates in warm water)

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
	}

	if (!inWater)
	{
		// Evaporates quickly when beached
		if (sim->rng.chance(1, 20))
		{
			//@ JLLY (no water) -> WTRV (desiccates)
			sim->part_change_type(i, x, y, PT_WTRV);
			return 1;
		}
		return 0;
	}

	// Bell pulse: periodic upward burst simulating jellyfish propulsion
	if (sim->rng.chance(1, 35))
	{
		parts[i].vy -= 1.2f; // upward pulse
		parts[i].vx += (sim->rng.between(-5, 5)) * 0.08f; // gentle lateral drift
	}

	// Dampen horizontal velocity (drag in water)
	parts[i].vx *= 0.88f;

	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	// Translucent blue-white bioluminescent glow
	*colr = 0x90;
	*colg = 0xD8;
	*colb = 0xFF;
	*firea = 70;
	*firer = 0x40;
	*fireg = 0xC0;
	*fireb = 0xFF;
	*pixel_mode |= FIRE_ADD | PMODE_BLUR;
	return 0;
}
