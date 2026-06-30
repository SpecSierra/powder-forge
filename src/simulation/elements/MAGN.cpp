#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_MAGN()
{
	Identifier = "DEFAULT_PT_MAGN";
	Name = "Magnet";
	Colour = 0xC02878_rgb;
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
	Hardness = 0;

	Weight = 100;

	HeatConduct = 100;
	Description = "Magnet. Attracts iron powder (BRMT). Demagnetizes above 1043K (Curie point) into iron.";

	Properties = TYPE_SOLID | PROP_CONDUCTS | PROP_HOT_GLOW;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 1043.0f;
	HighTemperatureTransition = PT_IRON; //@ MAGN -> IRON (demagnetizes at Curie temp)

	Update = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
{
	// Attract ferrous powder particles toward the magnet
	for (auto rx = -4; rx <= 4; rx++)
	for (auto ry = -4; ry <= 4; ry++)
	{
		if (!rx && !ry) continue;
		auto r = pmap[y+ry][x+rx];
		if (!r) continue;

		if (TYP(r) == PT_BRMT && sim->rng.chance(1, 3))
		{
			//@ MAGN attracts BRMT (iron powder)
			float dist2 = float(rx*rx + ry*ry) + 1.0f;
			float force = 1.2f / dist2;
			parts[ID(r)].vx -= rx * force;
			parts[ID(r)].vy -= ry * force;
		}
		// Electromagnetic induction: sparks nearby conductors
		if ((TYP(r) == PT_METL || TYP(r) == PT_IRON || TYP(r) == PT_COPR)
			&& sim->rng.chance(1, 500))
		{
			//@ MAGN near METL/IRON/COPR -> SPRK (electromagnetic induction)
			auto nx = x + rx + sim->rng.between(-1,1);
			auto ny = y + ry + sim->rng.between(-1,1);
			sim->create_part(-1, nx, ny, PT_SPRK);
		}
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	// Magnetic field shimmer effect
	*firea = 60;
	*firer = 0xC0;
	*fireg = 0x28;
	*fireb = 0x78;
	*pixel_mode |= FIRE_ADD;
	return 0;
}
