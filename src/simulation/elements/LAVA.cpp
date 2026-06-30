#include "simulation/ElementCommon.h"
#include "FIRE.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_LAVA()
{
	Identifier = "DEFAULT_PT_LAVA";
	Name = "Lava";
	Colour = 0xE05010_rgb;
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;

	Advection = 0.3f;
	AirDrag = 0.02f * CFDS;
	AirLoss = 0.95f;
	Loss = 0.80f;
	Collision = 0.0f;
	Gravity = 0.15f;
	Diffusion = 0.00f;
	HotAir = 0.0003f	* CFDS;
	Falldown = 2;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 2;
	PhotonReflectWavelengths = 0x3FF00000;

	Weight = 45;

	DefaultProperties.temp = R_TEMP + 1500.0f + 273.15f;
	HeatConduct = 60;
	Description = "Molten lava. Ignites flammable materials. Generated when metals and other materials melt, solidifies when cold.";

	Properties = TYPE_LIQUID|PROP_LIFE_DEC;
	CarriesTypeIn = 1U << FIELD_CTYPE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = MAX_TEMP;// check for lava solidification at all temperatures
	LowTemperatureTransition = ST;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &update;
	Graphics = &graphics;
	Create = &create;
}

static int update(UPDATE_FUNC_ARGS)
{
	for (auto rx = -1; rx <= 1; rx++)
	for (auto ry = -1; ry <= 1; ry++)
	{
		if (!rx && !ry) continue;
		auto r = pmap[y+ry][x+rx];
		if (!r) continue;
		if ((TYP(r) == PT_WATR || TYP(r) == PT_DSTW || TYP(r) == PT_ICEI)
			&& sim->rng.chance(1, 50))
		{
			//@ LAVA + WATR/DSTW/ICEI -> 2xOBSN (quench into obsidian)
			sim->part_change_type(i, x, y, PT_OBSN);
			sim->part_change_type(ID(r), x+rx, y+ry, PT_OBSN);
			return 1;
		}
	}
	// Volcanic outgassing: lava releases sulfurous and sulfide gases
	if (sim->rng.chance(1, 3000))
		sim->create_part(-1, x + sim->rng.between(-1, 1), y - 1, PT_SO2); //@ LAVA -> SO2 (volcanic outgassing)
	if (sim->rng.chance(1, 5000))
		sim->create_part(-1, x + sim->rng.between(-1, 1), y - 1, PT_H2S); //@ LAVA -> H2S (volcanic hydrogen sulfide)
	return Element_FIRE_update(UPDATE_FUNC_SUBCALL_ARGS);
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	*colr = cpart->life * 2 + 0xE0;
	*colg = cpart->life * 1 + 0x50;
	*colb = cpart->life / 2 + 0x10;
	if (*colr>255) *colr = 255;
	if (*colg>192) *colg = 192;
	if (*colb>128) *colb = 128;
	*firea = 40;
	*firer = *colr;
	*fireg = *colg;
	*fireb = *colb;
	*pixel_mode |= FIRE_ADD;
	*pixel_mode |= PMODE_BLUR;
	//Returning 0 means dynamic, do not cache
	return 0;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	sim->parts[i].life = sim->rng.between(240, 359);
}
