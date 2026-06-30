#include "simulation/ElementCommon.h"
#include <algorithm>

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_WOOD()
{
	Identifier = "DEFAULT_PT_WOOD";
	Name = "Wood";
	Colour = 0xC0A040_rgb;
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
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 20;
	Explosive = 0;
	Meltable = 0;
	Hardness = 16;

	Weight = 100;

	HeatConduct = 164;
	Description = "Wood, flammable.";

	Properties = TYPE_SOLID | PROP_NEUTPENETRATE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 873.0f;
	HighTemperatureTransition = PT_FIRE; //@ WOOD -> FIRE

	Update = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
{
	if (parts[i].temp > 450 && parts[i].temp > parts[i].tmp)
		parts[i].tmp = (int)parts[i].temp;

	if (parts[i].temp > 773.0f && sim->pv[y/CELL][x/CELL] <= -10.0f)
	{
		float temp = parts[i].temp;
		//@ WOOD -> BCOL
		sim->create_part(i, x, y, PT_BCOL);
		parts[i].temp = temp;
	}

	// Pyrolysis: wood slowly charcoalifies when heated without direct flame
	if (parts[i].temp > 450.0f && parts[i].temp < 650.0f && sim->rng.chance(1, 300))
	{
		bool hasFire = false;
		for (auto rx = -1; rx <= 1 && !hasFire; rx++)
		for (auto ry = -1; ry <= 1 && !hasFire; ry++) {
			if (!rx && !ry) continue;
			auto r = pmap[y+ry][x+rx];
			if (r && TYP(r) == PT_FIRE) hasFire = true;
		}
		if (!hasFire)
		{
			//@ WOOD (450-650K, no FIRE) -> CHAR (pyrolysis / charcoal kiln)
			sim->part_change_type(i, x, y, PT_CHAR);
			return 1;
		}
	}

	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	float maxtemp = std::max((float)cpart->tmp, cpart->temp);
	if (maxtemp > 400)
	{
		*colr -= (int)restrict_flt((maxtemp-400)/3,0,172);
		*colg -= (int)restrict_flt((maxtemp-400)/4,0,140);
		*colb -= (int)restrict_flt((maxtemp-400)/20,0,44);
	}
	if (maxtemp < 273)
	{
		*colr -= (int)restrict_flt((273-maxtemp)/5,0,40);
		*colg += (int)restrict_flt((273-maxtemp)/4,0,40);
		*colb += (int)restrict_flt((273-maxtemp)/1.5,0,150);
	}
	return 0;
}
