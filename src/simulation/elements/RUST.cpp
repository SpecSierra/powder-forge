#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_RUST()
{
	Identifier = "DEFAULT_PT_RUST";
	Name = "Rust";
	Colour = 0xB04020_rgb;
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
	Enabled = 1;

	Advection = 0.4f;
	AirDrag = 0.04f * CFDS;
	AirLoss = 0.94f;
	Loss = 0.95f;
	Collision = -0.1f;
	Gravity = 0.3f;
	Diffusion = 0.00f;
	HotAir = 0.000f * CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 30;

	Weight = 90;

	HeatConduct = 100;
	Description = "Rust. Iron oxide. Seed near iron and water to corrode it. Thermite reaction with magnesium yields pure iron and intense heat.";

	Properties = TYPE_PART;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	bool hasWater = false;

	for (auto rx = -2; rx <= 2; rx++)
	for (auto ry = -2; ry <= 2; ry++)
	{
		if (!rx && !ry) continue;
		if (y+ry < 0 || y+ry >= YRES || x+rx < 0 || x+rx >= XRES) continue;
		auto r = pmap[y+ry][x+rx];
		if (!r) continue;

		if (TYP(r) == PT_WATR || TYP(r) == PT_SLTW || TYP(r) == PT_DSTW)
			hasWater = true;
	}

	for (auto rx = -1; rx <= 1; rx++)
	for (auto ry = -1; ry <= 1; ry++)
	{
		if (!rx && !ry) continue;
		auto r = pmap[y+ry][x+rx];
		if (!r) continue;

		// Viral corrosion: rust spreads to adjacent iron when wet
		if (TYP(r) == PT_IRON && hasWater && sim->rng.chance(1, 200))
		{
			//@ RUST + IRON + WATR -> RUST (corrosion spreads)
			sim->part_change_type(ID(r), x+rx, y+ry, PT_RUST);
		}
		// Also spreads to steel, but much more slowly
		if (TYP(r) == PT_STEL && hasWater && sim->rng.chance(1, 2000))
		{
			//@ RUST + STEL + WATR -> RUST (steel corrodes very slowly)
			sim->part_change_type(ID(r), x+rx, y+ry, PT_RUST);
		}

		// Thermite: rust + magnesium at high temperature = iron + intense heat
		if (TYP(r) == PT_MAGN && parts[i].temp > 700.0f && sim->rng.chance(1, 8))
		{
			//@ RUST + MAGN (>700K) -> IRON + DUST + extreme heat (thermite)
			parts[i].temp = 2800.0f; // extreme exotherm
			sim->part_change_type(i, x, y, PT_IRON);
			sim->create_part(-1, x + sim->rng.between(-1,1), y + sim->rng.between(-1,1), PT_DUST); // MgO ash
			sim->create_part(-1, x + sim->rng.between(-2,2), y + sim->rng.between(-2,2), PT_FIRE);
			sim->kill_part(ID(r)); // magnesium consumed
			return 1;
		}
	}
	return 0;
}
