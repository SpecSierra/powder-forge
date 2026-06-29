#include "QuickOptions.h"

#include "GameModel.h"
#include "GameController.h"

#include "simulation/Simulation.h"

SandEffectOption::SandEffectOption(GameModel * m):
QuickOption(String(char32_t(0xE01C)), "Sand effect", m, Toggle)
{
	iconColour = 0xFAB387_rgb; // Peach
}
bool SandEffectOption::GetToggle()
{
	return m->GetSimulation()->pretty_powder;
}
void SandEffectOption::perform()
{
	m->GetSimulation()->pretty_powder = !m->GetSimulation()->pretty_powder;
}



DrawGravOption::DrawGravOption(GameModel * m):
QuickOption(String(char32_t(0xE018)), "Draw gravity field \bg(ctrl+g)", m, Toggle)
{
	iconColour = 0x89DCEB_rgb; // Sky
}
bool DrawGravOption::GetToggle()
{
	return m->GetGravityGrid();
}
void DrawGravOption::perform()
{
	m->ShowGravityGrid(!m->GetGravityGrid());
}



DecorationsOption::DecorationsOption(GameModel * m):
QuickOption(String(char32_t(0xE05F)), "Draw decorations \bg(ctrl+b)", m, Toggle)
{
	iconColour = 0xCBA6F7_rgb; // Mauve
}
bool DecorationsOption::GetToggle()
{
	return m->GetDecoration();
}
void DecorationsOption::perform()
{
	m->SetDecoration(!m->GetDecoration());
}



NGravityOption::NGravityOption(GameModel * m):
QuickOption(String(char32_t(0xE019)), "Newtonian Gravity \bg(n)", m, Toggle)
{
	iconColour = 0xF9E2AF_rgb; // Yellow
}
bool NGravityOption::GetToggle()
{
	return m->GetNewtonianGrvity();
}
void NGravityOption::perform()
{
	m->SetNewtonianGravity(!m->GetNewtonianGrvity());
}



AHeatOption::AHeatOption(GameModel * m):
QuickOption(String(char32_t(0xE03E)), "Ambient heat \bg(u)", m, Toggle)
{
	iconColour = 0xF38BA8_rgb; // Red
}
bool AHeatOption::GetToggle()
{
	return m->GetAHeatEnable();
}
void AHeatOption::perform()
{
	m->SetAHeatEnable(!m->GetAHeatEnable());
}



ConsoleShowOption::ConsoleShowOption(GameModel * m, GameController * c_):
QuickOption(String(char32_t(0xE065)), "Show Console \bg(~)", m, Toggle)
{
	iconColour = 0xA6E3A1_rgb; // Green
	c = c_;
}
bool ConsoleShowOption::GetToggle()
{
	return 0;
}
void ConsoleShowOption::perform()
{
	c->ShowConsole();
}
