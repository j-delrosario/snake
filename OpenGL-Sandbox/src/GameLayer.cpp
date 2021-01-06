#include "GameLayer.h"
#include "Random.h"
#include "Defines.h"
using namespace GLCore;
using namespace GLCore::Utils;

GameLayer::GameLayer()
	: Layer("GameLayer")
{
	auto& window = Application::Get().GetWindow();
	
	CreateCamera(window.GetWidth(), window.GetHeight());
	m_Camera->SetPosition({ 0.0f, 0.0f, 0.0f });

	Renderer::SetClearColor({ 0.0f, 0.0f, 0.0f, 0.0f });
	Renderer::EnableBlend();

	Random::Init();
}

GameLayer::~GameLayer()
{
}

void GameLayer::OnAttach()
{
	EnableGLDebugging();

	m_Level.Init();
}

void GameLayer::OnDetach()
{
}

void GameLayer::OnEvent(Event& e)
{
	EventDispatcher dispatcher(e);
	dispatcher.Dispatch<WindowResizeEvent>(GLCORE_BIND_EVENT_FN(GameLayer::OnWindowResize));
	dispatcher.Dispatch<KeyPressedEvent>(GLCORE_BIND_EVENT_FN(GameLayer::OnKeyPressed));
}

void GameLayer::OnUpdate(Timestep ts)
{
	if (m_State == GameState::Play) {
		m_Level.OnUpdate(ts);
	}

	Renderer::Clear();

	Renderer::BeginScene(*m_Camera.get());

	Renderer::BeginBatch();
	m_Level.OnRender();
	Renderer::EndBatch();
	Renderer::Flush();

	Renderer::EndScene();
}

void GameLayer::OnImGuiRender()
{
}

bool GameLayer::OnWindowResize(WindowResizeEvent& e)
{
	CreateCamera(e.GetWidth(), e.GetHeight());
	return false;
}

bool GameLayer::OnKeyPressed(KeyPressedEvent& e)
{
	if (e.GetKeyCode() == HZ_KEY_SPACE && m_State == GameState::Pause)
		m_State = GameState::Play;
	return false;
}

void GameLayer::CreateCamera(uint32_t width, uint32_t height)
{
	float aspectRatio = float(width) / float(height);
	float bottom = -CAM_HEIGHT;
	float top = CAM_HEIGHT;
	float left = bottom * aspectRatio;
	float right = top * aspectRatio;

	m_Camera = std::make_unique<OrthographicCamera>(left, right, bottom, top);
}
