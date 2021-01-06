#include "Level.h"
#include "Random.h"
#include "Defines.h"

using namespace GLCore;
using namespace GLCore::Utils;

void Level::Init()
{
	Reset();
}

void Level::OnUpdate(Timestep ts)
{
	m_Tick += ts;
	if (m_Tick < 0.05f)
		return;


	m_Tick = 0.0f;
	m_Snake.OnUpdate(ts);

	if (CollisionTest()) 
	{
		m_Snake.Grow();
		Reset();
	}
}

void Level::OnRender()
{
	Renderer::DrawQuad(m_Apple.Position, m_Apple.Size, m_Apple.Color);

	m_Snake.OnRender();
}

void Level::OnImGuiRender()
{
}

bool Level::CollisionTest()
{
	return m_Snake.m_Head == m_Apple.Position;
}

void Level::Reset()
{
	float x = (Random::Int() % (static_cast<int>(CAM_WIDTH) * 2)) - CAM_WIDTH;
	float y = (Random::Int() % (static_cast<int>(CAM_HEIGHT) * 2)) - CAM_HEIGHT;
	m_Apple.Position = { x, y };
}
