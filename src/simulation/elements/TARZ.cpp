#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_TARZ()
{
	Identifier = "DEFAULT_PT_TARZ";
	Name = "Tar";
	Colour = 0x0C0808_rgb;
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;

	Advection = 0.1f;
	AirDrag = 0.03f * CFDS;
	AirLoss = 0.96f;
	Loss = 0.45f;
	Collision = 0.0f;
	Gravity = 0.04f;
	Diffusion = 0.00f;
	HotAir = 0.000f * CFDS;
	Falldown = 2;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 5;

	Weight = 40;

	HeatConduct = 18;
	Description = "Tar. Thick, black liquid from coal distillation. Extremely flammable, hard to extinguish.";

	Properties = TYPE_LIQUID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 620.0f;
	HighTemperatureTransition = PT_FIRE; //@ TARZ -> FIRE (ignites)

	DefaultProperties.life = 120;

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	if (parts[i].life > 0 && parts[i].life < 120)
	{
		// Actively burning: produce thick black smoke and super-heat surroundings
		parts[i].life--;
		if (sim->rng.chance(1, 4))
			sim->create_part(-1, x + sim->rng.between(-1,1), y - 1, PT_SMKE);
		if (sim->rng.chance(1, 10))
			sim->create_part(-1, x + sim->rng.between(-1,1), y + sim->rng.between(-1,1), PT_FIRE);
		parts[i].temp = 1800.0f;
		if (parts[i].life <= 0)
		{
			//@ TARZ burns out -> CO2
			sim->part_change_type(i, x, y, PT_CO2);
			return 1;
		}
		return 0;
	}

	for (auto rx = -1; rx <= 1; rx++)
	for (auto ry = -1; ry <= 1; ry++)
	{
		if (!rx && !ry) continue;
		auto r = pmap[y+ry][x+rx];
		if (!r) continue;

		if (TYP(r) == PT_FIRE && sim->rng.chance(1, 5))
		{
			//@ TARZ + FIRE -> burns (life countdown)
			parts[i].life = 119;
			return 0;
		}
		if (TYP(r) == PT_ACID && sim->rng.chance(1, 20))
		{
			//@ TARZ + ACID -> dissolves
			sim->kill_part(i);
			return 1;
		}
	}
	return 0;
}
