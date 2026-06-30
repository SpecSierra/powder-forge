#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_CHAR()
{
	Identifier = "DEFAULT_PT_CHAR";
	Name = "Charcoal";
	Colour = 0x181818_rgb;
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
	Hardness = 10;

	Weight = 100;

	HeatConduct = 180;
	Description = "Charcoal. Formed by slow pyrolysis of wood without flame (450-650K). Purer carbon than coal. Absorbs water. Makes steel with molten iron.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	DefaultProperties.life = 120;
	DefaultProperties.tmp2 = 0; // peak temp tracking (like coal)

	Update = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
{
	// Burning countdown (like coal)
	if (parts[i].life <= 0)
	{
		sim->create_part(i, x, y, PT_FIRE);
		return 1;
	}
	if (parts[i].life < 120)
	{
		parts[i].life--;
		if (sim->rng.chance(1, 3))
			sim->create_part(-1, x + sim->rng.between(-1,1), y + sim->rng.between(-1,1), PT_FIRE);
		if (sim->rng.chance(1, 600))
			sim->create_part(-1, x + sim->rng.between(-1,1), y - 1, PT_CO2);
		// Charcoal burns cleaner than coal — no SO2
	}

	for (auto rx = -1; rx <= 1; rx++)
	for (auto ry = -1; ry <= 1; ry++)
	{
		if (!rx && !ry) continue;
		auto r = pmap[y+ry][x+rx];
		if (!r) continue;

		// Blast furnace steelmaking: charcoal carburises molten iron
		if (TYP(r) == PT_LAVA && parts[ID(r)].ctype == PT_IRON
		    && parts[i].temp > 1500.0f && sim->rng.chance(1, 25))
		{
			//@ CHAR + IRON(LAVA) at >1500K -> STEL (charcoal steelmaking)
			sim->part_change_type(ID(r), x+rx, y+ry, PT_STEL);
			sim->kill_part(i); // charcoal consumed
			return 1;
		}

		// Activated charcoal filtration: absorbs water
		if (TYP(r) == PT_WATR && sim->rng.chance(1, 60))
		{
			//@ CHAR absorbs WATR (filtration / activated carbon)
			sim->kill_part(ID(r));
		}

		// ACID dissolves charcoal
		if (TYP(r) == PT_ACID && sim->rng.chance(1, 30))
		{
			//@ CHAR + ACID -> CO2 (carbon dissolved)
			sim->part_change_type(i, x, y, PT_CO2);
			return 1;
		}
	}

	if (parts[i].temp > parts[i].tmp2)
		parts[i].tmp2 = (int)parts[i].temp;

	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	// Glows orange-red like coal when very hot
	if (cpart->tmp2 > 400)
	{
		int hot = cpart->tmp2 - 400;
		*colr += hot / 4;
		if (*colr > 160) *colr = 160;
	}
	*colg = *colb = *colr;
	return 0;
}
