#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_SILV()
{
	Identifier = "DEFAULT_PT_SILV";
	Name = "Silver";
	Colour = 0xE0E0F0_rgb;
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag   = 0.00f * CFDS;
	AirLoss   = 0.90f;
	Loss      = 0.00f;
	Collision = 0.0f;
	Gravity   = 0.0f;
	Diffusion = 0.00f;
	HotAir    = 0.000f * CFDS;
	Falldown  = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable  = 1;
	Hardness  = 0; // acid-resistant

	Weight = 100;

	HeatConduct = 251;
	Description = "Silver. Best conductor. Tarnishes in salt water. Antibacterial — kills yeast on contact.";

	Properties = TYPE_SOLID | PROP_CONDUCTS | PROP_LIFE_DEC | PROP_HOT_GLOW;

	LowPressure            = IPL;
	LowPressureTransition  = NT;
	HighPressure           = IPH;
	HighPressureTransition = NT;
	LowTemperature            = ITL;
	LowTemperatureTransition  = NT;
	HighTemperature            = 1235.0f;
	HighTemperatureTransition  = PT_LAVA; //@ SILV -> LAVA(SILV)

	Update   = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
{
	// Fast conduction: check 4 pixels away like GOLD and PTNM
	if (!parts[i].life)
	{
		static const int checkX[] = { -4, 4, 0, 0 };
		static const int checkY[] = { 0, 0, -4, 4 };
		for (int j = 0; j < 4; j++)
		{
			auto r = pmap[y + checkY[j]][x + checkX[j]];
			if (r && TYP(r) == PT_SPRK && parts[ID(r)].life && parts[ID(r)].life < 4)
			{
				sim->part_change_type(i, x, y, PT_SPRK);
				parts[i].life = 4;
				parts[i].ctype = PT_SILV;
			}
		}
	}

	for (auto rx = -1; rx <= 1; rx++)
	for (auto ry = -1; ry <= 1; ry++)
	{
		if (!rx && !ry) continue;
		auto r = pmap[y+ry][x+rx];
		if (!r) continue;

		if ((TYP(r) == PT_SLTW || TYP(r) == PT_SALT) && sim->rng.chance(1, 500))
		{
			//@ SILV + SLTW/SALT -> BMTL (tarnishes — silver chloride)
			sim->part_change_type(i, x, y, PT_BMTL);
			return 1;
		}
		if (TYP(r) == PT_YEST && sim->rng.chance(1, 50))
		{
			//@ SILV + YEST -> (antibacterial: silver ions kill yeast)
			sim->kill_part(ID(r));
		}
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	// Bright metallic shimmer
	int rndstore = gfctx.rng.gen();
	int shine = (rndstore % 14) - 7;
	*colr += shine;
	*colg += shine;
	*colb += shine;
	return 0;
}
