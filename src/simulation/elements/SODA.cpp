#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_SODA()
{
	Identifier = "DEFAULT_PT_SODA";
	Name = "Sodium";
	Colour = 0xC8C8D8_rgb;
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
	Meltable = 1;
	Hardness = 0; // reactions handled in update

	Weight = 100;

	HeatConduct = 200;
	Description = "Sodium metal. Reacts violently with water, producing caustic soda (NaOH) and hydrogen gas. Reacts with chlorine gas to form salt.";

	Properties = TYPE_SOLID | PROP_HOT_GLOW;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 371.0f; // sodium melts at 98°C
	HighTemperatureTransition = PT_LAVA; //@ SODA -> LAVA (sodium melts, ctype=SODA)

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

		// Violent reaction with any water — THE classic chemistry demo
		if (TYP(r) == PT_WATR || TYP(r) == PT_DSTW || TYP(r) == PT_SLTW)
		{
			if (sim->rng.chance(1, 3))
			{
				//@ SODA + WATR -> CAUS + H2 + FIRE (2Na + 2H2O → 2NaOH + H2, extremely exothermic)
				parts[i].temp = 1000.0f; // violent exotherm
				sim->part_change_type(i, x, y, PT_CAUS); // NaOH
				sim->part_change_type(ID(r), x+rx, y+ry, PT_H2); // hydrogen gas released
				sim->create_part(-1, x + sim->rng.between(-1,1), y - 1, PT_FIRE); // heat burst
				return 1;
			}
			return 0;
		}

		// Reacts with chlorine gas → common salt (NaCl)
		if (TYP(r) == PT_CL2 && sim->rng.chance(1, 5))
		{
			//@ SODA + CL2 -> SALT (2Na + Cl2 → 2NaCl)
			sim->part_change_type(i, x, y, PT_SALT);
			sim->kill_part(ID(r)); // chlorine consumed
			return 1;
		}

		// Reacts with acid — very fast neutralisation
		if (TYP(r) == PT_ACID && sim->rng.chance(1, 2))
		{
			//@ SODA + ACID -> CAUS + H2 (sodium + acid → salt + hydrogen)
			sim->part_change_type(i, x, y, PT_CAUS);
			sim->create_part(-1, x + sim->rng.between(-1,1), y - 1, PT_H2);
			return 1;
		}

		// Slow oxidation in air (forms white sodium oxide over time)
		if (TYP(r) == PT_O2 && sim->rng.chance(1, 500))
		{
			//@ SODA + O2 -> DUST (sodium oxide, Na2O, white powder)
			sim->part_change_type(i, x, y, PT_DUST);
			return 1;
		}
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	// Bright silvery metal — freshly cut sodium is shiny
	*colr = 0xC8;
	*colg = 0xC8;
	*colb = 0xD8;
	return 0;
}
