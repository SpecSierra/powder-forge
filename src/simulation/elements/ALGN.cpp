#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_ALGN()
{
	Identifier = "DEFAULT_PT_ALGN";
	Name = "Algae";
	Colour = 0x20A060_rgb;
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;

	Advection = 0.6f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.98f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.05f;
	Diffusion = 0.00f;
	HotAir = 0.000f * CFDS;
	Falldown = 2;

	Flammable = 10;
	Explosive = 0;
	Meltable = 0;
	Hardness = 5;

	Weight = 29; // lighter than water — floats

	HeatConduct = 40;
	Description = "Algae. Floats on water. Converts CO2 to O2 (photosynthesis). Ferments with yeast into alcohol.";

	Properties = TYPE_LIQUID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 270.0f;
	LowTemperatureTransition = PT_ICEI; //@ ALGN -> ICEI (freezes)
	HighTemperature = 355.0f;
	HighTemperatureTransition = PT_DUST; //@ ALGN -> DUST (dries out)

	Update = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
{
	for (auto rx = -2; rx <= 2; rx++)
	for (auto ry = -2; ry <= 2; ry++)
	{
		if (!rx && !ry) continue;
		auto r = pmap[y+ry][x+rx];
		if (!r) continue;

		// Photosynthesis: consume CO2, produce O2
		if (TYP(r) == PT_CO2
			&& parts[i].temp > 273.0f && parts[i].temp < 350.0f
			&& sim->rng.chance(1, 80))
		{
			//@ ALGN + CO2 (273-350K) -> O2 (photosynthesis)
			sim->part_change_type(ID(r), x+rx, y+ry, PT_O2);

			// Slow growth: occasionally create new algae near water
			if (sim->rng.chance(1, 10))
			{
				int nx = x + sim->rng.between(-2, 2);
				int ny = y + sim->rng.between(-2, 2);
				auto nr = pmap[ny][nx];
				if (nr && TYP(nr) == PT_WATR)
					sim->part_change_type(ID(nr), nx, ny, PT_ALGN); //@ ALGN grows in WATR + CO2
			}
		}

		// Fermentation with yeast
		if (TYP(r) == PT_YEST
			&& parts[i].temp > 283.0f && parts[i].temp < 360.0f
			&& sim->rng.chance(1, 200))
		{
			//@ ALGN + YEST (283-360K) -> ALCH + CO2 (algae fermentation)
			sim->part_change_type(i, x, y, PT_ALCH);
			sim->create_part(-1, x + sim->rng.between(-1,1), y - 1, PT_CO2);
			return 1;
		}

		if (TYP(r) == PT_ACID && sim->rng.chance(1, 10))
		{
			//@ ALGN + ACID -> dies
			sim->kill_part(i);
			return 1;
		}
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	// Subtle green shimmer
	*firea = 25;
	*firer = 0x10;
	*fireg = 0xC0;
	*fireb = 0x30;
	*pixel_mode |= FIRE_ADD;
	return 0;
}
