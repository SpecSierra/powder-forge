#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_PHOS()
{
	Identifier = "DEFAULT_PT_PHOS";
	Name = "Phosphorus";
	Colour = 0xD4E020_rgb;
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
	HotAir = 0.002f * CFDS; // warms air slightly (oxidation)
	Falldown = 1;

	Flammable = 80;
	Explosive = 1;
	Meltable = 0;
	Hardness = 1;

	Weight = 85;

	HeatConduct = 60;
	Description = "Phosphorus. White phosphorus. Glows yellow-green. Auto-ignites at 310K (37°C) in air. Reacts violently with oxygen.";

	Properties = TYPE_PART;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 310.0f; // auto-ignites at 37°C
	HighTemperatureTransition = PT_FIRE; //@ PHOS -> FIRE (spontaneous combustion)

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

		// Rapid oxidation in oxygen-rich environment
		if (TYP(r) == PT_O2 && sim->rng.chance(1, 30))
		{
			//@ PHOS + O2 -> FIRE + CO2 (violent oxidation: 4P + 5O2 → P4O10)
			parts[i].temp += 200.0f; // exothermic
			sim->part_change_type(i, x, y, PT_FIRE);
			sim->kill_part(ID(r)); // O2 consumed
			return 1;
		}
		// Water provides some protection but also reacts (produces phosphine-like gas)
		if ((TYP(r) == PT_WATR || TYP(r) == PT_DSTW) && sim->rng.chance(1, 150))
		{
			//@ PHOS + WATR -> H2S-like gas (phosphine approximation, toxic)
			sim->create_part(-1, x + sim->rng.between(-1,1), y - 1, PT_H2S);
			parts[i].temp += 10.0f;
		}
		if (TYP(r) == PT_ACID && sim->rng.chance(1, 5))
		{
			//@ PHOS + ACID -> dissolves rapidly
			sim->kill_part(i);
			return 1;
		}
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	// Constant bioluminescent yellow-green glow — phosphorus glows in the dark
	*colr = 0xD4;
	*colg = 0xE0;
	*colb = 0x20;
	*firea = 60;
	*firer = 0x80;
	*fireg = 0xFF;
	*fireb = 0x20;
	*pixel_mode |= FIRE_ADD;
	return 0;
}
