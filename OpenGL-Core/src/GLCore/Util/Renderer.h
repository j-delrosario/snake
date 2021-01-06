#pragma once

#include "GLCore/Core/Timestep.h"
#include "../Events/Event.h"
#include "OrthographicCamera.h"
#include <glm/glm.hpp>

namespace GLCore::Utils {
	// 2D Batch Renderer
	class Renderer
	{
	public:
		static void Init();
		static void Shutdown();

		// Deprecated setup and update for the camera -- tbremoved
		static void Start();
		static void OnUpdate(Timestep ts);
		static void OnEvent(Event& event);

		// New camera update functions
		static void BeginScene(const OrthographicCamera& camera);
		static void EndScene();

		static void BeginBatch();
		static void EndBatch();
		static void Flush();

		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, uint32_t textureID);

		static void SetClearColor(const glm::vec4& color);
		static void Clear();
		static void EnableBlend();

		// Stats
		struct Stats
		{
			uint32_t DrawCount = 0;
			uint32_t QuadCount = 0;
		};

		static const Stats& GetStats();
		static void ResetStats();
		static void SetUniformMat4(uint32_t shader, const char* name, const glm::mat4& matrix);
	};
}