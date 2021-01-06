#pragma once

#include <GLCore.h>
#include <GLCoreUtils.h>

#include "Level.h"

using namespace GLCore;
using namespace GLCore::Utils;

class GameLayer : public Layer
{
public:
	GameLayer();
	virtual ~GameLayer();

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnEvent(Event& e) override;
	virtual void OnUpdate(Timestep ts) override;
	virtual void OnImGuiRender() override;

	bool OnWindowResize(WindowResizeEvent& e);
	bool OnKeyPressed(KeyPressedEvent& e);
private:
	void CreateCamera(uint32_t width, uint32_t height);

private:
	std::unique_ptr<OrthographicCamera> m_Camera;
	Level m_Level;
	enum class GameState {
		Play = 0, Pause = 1
	};
	GameState m_State = GameState::Pause;
};