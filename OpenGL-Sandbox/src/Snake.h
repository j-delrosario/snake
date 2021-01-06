#pragma once

#include <GLCore.h>
#include <GLCoreUtils.h>

using namespace GLCore;
using namespace GLCore::Utils;

class Snake
{
public:
	Snake();

	void OnUpdate(Timestep ts);
	void OnRender();

	void Grow();

public:
	glm::vec2 m_Head;

private:
	std::vector<glm::vec2> m_Body;
	glm::vec2 m_Tail;
	glm::vec2 m_Velocity;
	uint32_t m_BaseSize;
};