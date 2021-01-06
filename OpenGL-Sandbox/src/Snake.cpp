#include "Snake.h"
#include "Defines.h"
using namespace GLCore;
using namespace GLCore::Utils;

Snake::Snake()
{
	m_Head = { 0.0f, 0.0f };
	m_Body = std::vector<glm::vec2>(5, {0.0f, 0.0f});
	m_Tail = { 0.0f, 0.0f };

	m_Velocity = { 0.0f, 0.0f };
	m_BaseSize = 5;
}

void Snake::OnUpdate(Timestep ts)
{
	std::vector<glm::vec2>::iterator it;
	it = m_Body.begin();
	float vx = 0, vy = 0;
	if (Input::IsKeyPressed(HZ_KEY_LEFT))
	{
		vx = -1.0f;
		vy =  0.0f;
	} else if (Input::IsKeyPressed(HZ_KEY_UP))
	{
		vx = 0.0f;
		vy = 1.0f;
	} else if (Input::IsKeyPressed(HZ_KEY_RIGHT))
	{
		vx = 1.0f;
		vy = 0.0f;
	} else if (Input::IsKeyPressed(HZ_KEY_DOWN))
	{
		vx =  0.0f;
		vy = -1.0f;
	}

	// Check if no input or turning back on itself
	if ((m_Body[0].x != m_Head.x + vx || m_Body[0].y != m_Head.y + vy) &&
		(m_Body[1].x != m_Head.x + vx || m_Body[1].y != m_Head.y + vy))
		m_Velocity = { vx, vy };

	float px = m_Head.x + m_Velocity.x, py = m_Head.y + m_Velocity.y;

	if (px > CAM_WIDTH)
		px = -CAM_WIDTH;
	else if (px < -CAM_WIDTH)
		px = CAM_WIDTH;
	else if (py > CAM_HEIGHT)
		py = -CAM_HEIGHT;
	else if (py < -CAM_HEIGHT)
		py = CAM_HEIGHT;

	for (glm::vec2 element : m_Body)
	{
		if (element.x == px && element.y == py)
			while (m_Body.size() > m_BaseSize)
				m_Body.pop_back();
	}
	m_Body.pop_back();
	m_Body.insert(it, { px, py });

	m_Head = m_Body.front();
	m_Tail = m_Body.back();


}

void Snake::OnRender()
{
	for (int i = 0; i < m_Body.size(); i++)
		Renderer::DrawQuad(m_Body[i], { 0.9f, 0.9f }, {1.0f, 1.0f, 1.0f, 1.0f});
}

void Snake::Grow()
{
	m_Body.push_back(m_Tail);
}
