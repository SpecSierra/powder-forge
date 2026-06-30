#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_OZON()
{
	Identifier = "DEFAULT_PT_OZON";
	Name = "Ozone";
	Colour = 0x8080E8_rgb;
	MenuVisible = 1;
	MenuSection = SC_GAS;
	Enabled = 1;

	Advection = 2.0f;
	AirDrag   = 0.00f * CFDS;
	AirLoss   = 0.99f;
	Loss      = 0.30f;
	Collision = -0.1f;
	Gravity   = 0.02f;
	Diffusion = 1.5f;
	HotAir    = 0.000f * CFDS;
	Falldown  = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable  = 0;
	Hardness  = 0;

	Weight = 1;

	HeatConduct = 70;
	Description = "Ozone. Strong oxidiser. Degrades rubber, wood, and plant matter. Kills yeast. Decomposes above 350K.";

	Properties = TYPE_GAS | PROP_DEADLY;

	LowPressure            = IPL;
	LowPressureTransition  = NT;
	HighPressure           = IPH;
	HighPressureTransition = NT;
	LowTemperature            = ITL;
	LowTemperatureTransition  = NT;
	HighTemperature            = 350.0f;
	HighTemperatureTransition  = PT_O2; //@ OZON -> O2 (thermal decomposition)

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	for (auto rx = -1; rx <= 1; rx++)
	for (auto ry = -1; ry <= 1; ry++)
	{
		if (!rx && !ry) continue;
		auto r = pmap[y+ry][x+rx];
		if (!r) continue;

		if (TYP(r) == PT_SMKE && sim->rng.chance(1, 50))
		{
			//@ OZON + SMKE -> CO2 (oxidises smoke particles)
			sim->part_change_type(ID(r), x+rx, y+ry, PT_CO2);
		}
		if ((TYP(r) == PT_WOOD || TYP(r) == PT_PLNT) && sim->rng.chance(1, 300))
		{
			//@ OZON + WOOD/PLNT -> BCOL (ozone degrades organic material)
			sim->part_change_type(ID(r), x+rx, y+ry, PT_BCOL);
		}
		if (TYP(r) == PT_RUBB && sim->rng.chance(1, 200))
		{
			//@ OZON + RUBB -> BMTL (ozone cracks and degrades rubber)
			sim->part_change_type(ID(r), x+rx, y+ry, PT_BMTL);
		}
		if (TYP(r) == PT_YEST && sim->rng.chance(1, 50))
		{
			//@ OZON + YEST -> (ozone disinfects — kills yeast)
			sim->kill_part(ID(r));
		}
		if (TYP(r) == PT_O2 && sim->rng.chance(1, 500))
		{
			//@ OZON spreads: O2 near OZON + PLSM -> more OZON
			auto ep = sim->photons[y+ry][x+rx];
			if (ep && TYP(ep) == PT_ELEC)
			{
				//@ O2 + ELEC (near OZON) -> OZON (ozone generation cascade)
				sim->part_change_type(ID(r), x+rx, y+ry, PT_OZON);
			}
		}
	}
	return 0;
}
