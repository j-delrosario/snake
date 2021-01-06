#pragma once

#include <GLCore.h>
#include <GLCoreUtils.h>

#include "Snake.h"

using namespace GLCore;
using namespace GLCore::Utils;

struct Apple
{
public:
	glm::vec2 Position = { -5.0f, -5.0f };
	glm::vec2 Size = { 0.9f, 0.9f };
	glm::vec4 Color = { 1.0f, 0.0f, 0.0f, 1.0f };
};

class Level
{
public:
	void Init();
	void OnUpdate(Timestep ts);
	void OnRender();
	void OnImGuiRender();
	void Reset();
private:
	bool CollisionTest();
private:
	Snake m_Snake;
	Apple m_Apple;
	bool  m_Eaten = false;
	float m_Tick  = 0.0f;
};