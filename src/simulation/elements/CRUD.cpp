#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_CRUD()
{
	Identifier = "DEFAULT_PT_CRUD";
	Name = "Crude Oil";
	Colour = 0x1A0A04_rgb;
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;

	Advection = 0.4f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.98f;
	Loss = 0.85f;
	Collision = 0.0f;
	Gravity = 0.10f;
	Diffusion = 0.00f;
	HotAir = 0.000f * CFDS;
	Falldown = 2;

	Flammable = 0; // burning handled manually — produces residue instead of vanishing
	Explosive = 0;
	Meltable = 0;
	Hardness = 2;

	Weight = 25; // heavier than refined OIL(20), still floats on WATR(30)

	HeatConduct = 20;
	Description = "Crude Oil. Raw petroleum. Floats on water. Distills into gas and diesel when heated. Burns to tar.";

	Properties = TYPE_LIQUID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 750.0f;
	HighTemperatureTransition = PT_FIRE; //@ CRUD -> FIRE (auto-ignites at high temp)

	DefaultProperties.life = 0; // 0 = not burning; >0 = burn countdown

	Update = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
{
	// --- Burning phase ---
	if (parts[i].life > 0)
	{
		parts[i].life--;

		// Emit black smoke and sulfurous gas while burning
		if (sim->rng.chance(1, 3))
			sim->create_part(-1, x + sim->rng.between(-1,1), y - 1, PT_SMKE);
		if (sim->rng.chance(1, 20))
			sim->create_part(-1, x + sim->rng.between(-1,1), y + sim->rng.between(-1,1), PT_SO2);
		if (sim->rng.chance(1, 25))
			sim->create_part(-1, x + sim->rng.between(-1,1), y + sim->rng.between(-1,1), PT_FIRE);

		parts[i].temp = 900.0f; // burning crude is very hot

		if (parts[i].life <= 0)
		{
			//@ CRUD (burns out) -> TARZ (heavy tar residue)
			sim->part_change_type(i, x, y, PT_TARZ);
			return 1;
		}
		return 0;
	}

	// --- Thermal distillation (not burning) ---
	float t = parts[i].temp;

	if (t > 350.0f && t < 520.0f && sim->rng.chance(1, 500))
	{
		//@ CRUD (350-520K) -> GAS (light fraction vaporises — natural gas / naphtha)
		sim->create_part(-1, x + sim->rng.between(-1,1), y - 1, PT_GAS);
	}
	if (t > 520.0f && t < 700.0f && sim->rng.chance(1, 400))
	{
		//@ CRUD (520-700K) -> DESL (middle distillate — diesel / kerosene fraction)
		sim->part_change_type(i, x, y, PT_DESL);
		return 1;
	}

	// --- Neighbour reactions ---
	for (auto rx = -1; rx <= 1; rx++)
	for (auto ry = -1; ry <= 1; ry++)
	{
		if (!rx && !ry) continue;
		auto r = pmap[y+ry][x+rx];
		if (!r) continue;

		if (TYP(r) == PT_FIRE && sim->rng.chance(1, 12))
		{
			//@ CRUD + FIRE -> starts burning (life countdown to TARZ)
			parts[i].life = 90;
			return 0;
		}
		if (TYP(r) == PT_ACID && sim->rng.chance(1, 25))
		{
			//@ CRUD + ACID -> TARZ (acid treatment produces heavy bitumen)
			sim->part_change_type(i, x, y, PT_TARZ);
			return 1;
		}
		// Underground seepage: crude oil slowly seeps through porous rock
		if ((TYP(r) == PT_STNE || TYP(r) == PT_ROCK) && sim->pv[y/CELL][x/CELL] > 5.0f && sim->rng.chance(1, 3000))
		{
			//@ CRUD seeps through STNE/ROCK under pressure (oil migration)
			sim->part_change_type(ID(r), x+rx, y+ry, PT_CRUD);
		}
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	// Subtle dark sheen — crude oil has a slight iridescent surface
	if (cpart->life > 0)
	{
		// Glowing orange when burning
		*firea = 80;
		*firer = 0xFF;
		*fireg = 0x60;
		*fireb = 0x10;
		*pixel_mode |= FIRE_ADD;
	}
	return 0;
}
