#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_NAPE()
{
	Identifier = "DEFAULT_PT_NAPE";
	Name = "Napalm";
	Colour = 0xFF4808_rgb;
	MenuVisible = 1;
	MenuSection = SC_EXPLOSIVE;
	Enabled = 1;

	Advection = 0.3f;
	AirDrag = 0.02f * CFDS;
	AirLoss = 0.96f;
	Loss = 0.80f;
	Collision = 0.0f;
	Gravity = 0.10f;
	Diffusion = 0.00f;
	HotAir = 0.000f * CFDS;
	Falldown = 2;

	Flammable = 300;
	Explosive = 0;
	Meltable = 0;
	Hardness = 2;

	Weight = 18; // lighter than water and oil — floats on everything

	HeatConduct = 42;
	Description = "Napalm. Incendiary gel. Floats on water, cannot be extinguished by water. Burns at extreme temperature.";

	Properties = TYPE_LIQUID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 400.0f;
	HighTemperatureTransition = PT_FIRE; //@ NAPE -> FIRE (self-ignites at low temp)

	Update = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
{
	for (auto rx = -1; rx <= 1; rx++)
	for (auto ry = -1; ry <= 1; ry++)
	{
		if (!rx && !ry) continue;
		auto r = pmap[y+ry][x+rx];
		if (!r) continue;

		// Napalm repels water — it burns on water and cannot be extinguished
		if ((TYP(r) == PT_WATR || TYP(r) == PT_DSTW) && sim->rng.chance(1, 15))
		{
			//@ NAPE + WATR -> WATR killed (napalm is hydrophobic)
			sim->kill_part(ID(r));
		}

		// Amplify nearby fire to extreme temperature
		if (TYP(r) == PT_FIRE && sim->rng.chance(1, 4))
		{
			//@ NAPE + FIRE -> super-hot fire (2500K+)
			parts[ID(r)].temp = 2500.0f;
			// Spread napalm as fire
			if (sim->rng.chance(1, 3))
			{
				sim->part_change_type(i, x, y, PT_FIRE);
				parts[i].temp = 2500.0f;
				return 1;
			}
		}
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	// Intense orange-red glow
	*firea = 120;
	*firer = 0xFF;
	*fireg = 0x50;
	*fireb = 0x08;
	*pixel_mode |= FIRE_ADD | PMODE_BLUR;
	return 0;
}
