#include "glpch.h"
#include "Renderer.h"
#include "Shader.h"
#include "OrthographicCameraController.h"

#include <array>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
namespace GLCore::Utils {
	static const size_t MaxQuadCount = 20000;
	static const size_t MaxVertexCount = MaxQuadCount * 4;
	static const size_t MaxIndexCount = MaxQuadCount * 6;
	static const size_t MaxTextures = 32;

	struct Vertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoords;
		float TexIndex;
	};

	struct RendererData
	{
		GLuint QuadVA = 0;
		GLuint QuadVB = 0;
		GLuint QuadIB = 0;

		uint32_t IndexCount = 0;

		Vertex* QuadBuffer = nullptr;
		Vertex* QuadBufferPtr = nullptr;

		GLuint WhiteTexture = 0;
		uint32_t WhiteTextureSlot = 0;

		std::array<uint32_t, MaxTextures> TextureSlots;
		uint32_t TextureSlotIndex = 1;

		Renderer::Stats RenderStats;

		OrthographicCameraController CameraController = OrthographicCameraController(16.0f / 9.0f);
		std::unique_ptr<Shader> Shader;
	};

	static RendererData s_Data;

	void Renderer::Init()
	{
		s_Data.QuadBuffer = new Vertex[MaxVertexCount];

		glCreateVertexArrays(1, &s_Data.QuadVA);
		glBindVertexArray(s_Data.QuadVA);

		glCreateBuffers(1, &s_Data.QuadVB);
		glBindBuffer(GL_ARRAY_BUFFER, s_Data.QuadVB);
		glBufferData(GL_ARRAY_BUFFER, MaxVertexCount * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

		glEnableVertexArrayAttrib(s_Data.QuadVA, 0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Position));

		glEnableVertexArrayAttrib(s_Data.QuadVA, 1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Color));

		glEnableVertexArrayAttrib(s_Data.QuadVA, 2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, TexCoords));

		glEnableVertexArrayAttrib(s_Data.QuadVA, 3);
		glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, TexIndex));

		uint32_t indices[MaxIndexCount];
		uint32_t offset = 0;
		uint32_t pattern[] = { 0, 1, 2, 2, 3, 0 };

		for (int i = 0; i < MaxIndexCount; i++) {
			if (i != 0 && i % 6 == 0)
				offset += 4;
			indices[i] = offset + pattern[i % 6];
		}

		glCreateBuffers(1, &s_Data.QuadIB);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_Data.QuadIB);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		// 1x1 white texture
		glCreateTextures(GL_TEXTURE_2D, 1, &s_Data.WhiteTexture);
		glBindTexture(GL_TEXTURE_2D, s_Data.WhiteTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		uint32_t color = 0xffffffff;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &color);

		memset(&s_Data.TextureSlots, 0, sizeof(s_Data.TextureSlots));
		s_Data.TextureSlots[0] = s_Data.WhiteTexture;

		s_Data.Shader = std::unique_ptr<Shader>(Shader::FromGLSLTextFiles(
			"assets/shaders/Renderer.vert.glsl",
			"assets/shaders/Renderer.frag.glsl"
		));
		glUseProgram(s_Data.Shader->GetRendererID());
		auto loc = glGetUniformLocation(s_Data.Shader->GetRendererID(), "u_Textures");
		int samplers[32];
		for (int i = 0; i < 32; i++) {
			samplers[i] = i;
		}
		glUniform1iv(loc, 32, samplers);

		EnableBlend();
	}

	void Renderer::Shutdown()
	{
		glDeleteVertexArrays(1, &s_Data.QuadVA);
		glDeleteBuffers(1, &s_Data.QuadVB);
		glDeleteBuffers(1, &s_Data.QuadIB);

		delete[] s_Data.QuadBuffer;
	}

	void Renderer::BeginBatch()
	{
		s_Data.QuadBufferPtr = s_Data.QuadBuffer;
	}

	void Renderer::EndBatch()
	{
		GLsizeiptr size = (uint8_t*)s_Data.QuadBufferPtr - (uint8_t*)s_Data.QuadBuffer;
		glBindBuffer(GL_ARRAY_BUFFER, s_Data.QuadVB);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, s_Data.QuadBuffer);
	}

	void Renderer::Flush()
	{
		for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++) {
			glBindTextureUnit(i, s_Data.TextureSlots[i]);
		}

		glBindVertexArray(s_Data.QuadVA);
		glDrawElements(GL_TRIANGLES, s_Data.IndexCount, GL_UNSIGNED_INT, nullptr);
		s_Data.RenderStats.DrawCount++;

		s_Data.IndexCount = 0;
		s_Data.TextureSlotIndex = 1;
	}

	void Renderer::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		if (s_Data.IndexCount >= MaxIndexCount) {
			EndBatch();
			Flush();
			BeginBatch();
		}

		float textureIndex = 0.0f;

		s_Data.QuadBufferPtr->Position = { position.x, position.y, 0.0f };
		s_Data.QuadBufferPtr->Color = color;
		s_Data.QuadBufferPtr->TexCoords = { 0.0f, 0.0f };
		s_Data.QuadBufferPtr->TexIndex = textureIndex;
		s_Data.QuadBufferPtr++;

		s_Data.QuadBufferPtr->Position = { position.x + size.x, position.y, 0.0f };
		s_Data.QuadBufferPtr->Color = color;
		s_Data.QuadBufferPtr->TexCoords = { 1.0f, 0.0f };
		s_Data.QuadBufferPtr->TexIndex = textureIndex;
		s_Data.QuadBufferPtr++;

		s_Data.QuadBufferPtr->Position = { position.x + size.x, position.y + size.y, 0.0f };
		s_Data.QuadBufferPtr->Color = color;
		s_Data.QuadBufferPtr->TexCoords = { 1.0f, 1.0f };
		s_Data.QuadBufferPtr->TexIndex = textureIndex;
		s_Data.QuadBufferPtr++;

		s_Data.QuadBufferPtr->Position = { position.x, position.y + size.y, 0.0f };
		s_Data.QuadBufferPtr->Color = color;
		s_Data.QuadBufferPtr->TexCoords = { 0.0f, 1.0f };
		s_Data.QuadBufferPtr->TexIndex = textureIndex;
		s_Data.QuadBufferPtr++;

		s_Data.IndexCount += 6;
		s_Data.RenderStats.QuadCount++;
	}

	void Renderer::DrawQuad(const glm::vec2& position, const glm::vec2& size, uint32_t textureID)
	{
		if (s_Data.IndexCount >= MaxIndexCount || s_Data.TextureSlotIndex > 31) {
			EndBatch();
			Flush();
			BeginBatch();
		}

		constexpr glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

		float textureIndex = 0.0f;
		for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
		{
			if (s_Data.TextureSlots[i] == textureID)
			{
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f) {
			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex] = textureID;
			s_Data.TextureSlotIndex++;
		}

		s_Data.QuadBufferPtr->Position = { position.x, position.y, 0.0f };
		s_Data.QuadBufferPtr->Color = color;
		s_Data.QuadBufferPtr->TexCoords = { 0.0f, 0.0f };
		s_Data.QuadBufferPtr->TexIndex = textureIndex;
		s_Data.QuadBufferPtr++;

		s_Data.QuadBufferPtr->Position = { position.x + size.x, position.y, 0.0f };
		s_Data.QuadBufferPtr->Color = color;
		s_Data.QuadBufferPtr->TexCoords = { 1.0f, 0.0f };
		s_Data.QuadBufferPtr->TexIndex = textureIndex;
		s_Data.QuadBufferPtr++;

		s_Data.QuadBufferPtr->Position = { position.x + size.x, position.y + size.y, 0.0f };
		s_Data.QuadBufferPtr->Color = color;
		s_Data.QuadBufferPtr->TexCoords = { 1.0f, 1.0f };
		s_Data.QuadBufferPtr->TexIndex = textureIndex;
		s_Data.QuadBufferPtr++;

		s_Data.QuadBufferPtr->Position = { position.x, position.y + size.y, 0.0f };
		s_Data.QuadBufferPtr->Color = color;
		s_Data.QuadBufferPtr->TexCoords = { 0.0f, 1.0f };
		s_Data.QuadBufferPtr->TexIndex = textureIndex;
		s_Data.QuadBufferPtr++;

		s_Data.IndexCount += 6;
		s_Data.RenderStats.QuadCount++;
	}

	void Renderer::SetClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void Renderer::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Renderer::EnableBlend()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);
	}

	void Renderer::Start()
	{
		s_Data.CameraController.Start();

		auto vp = s_Data.CameraController.GetCamera().GetViewProjectionMatrix();
		Renderer::SetUniformMat4(s_Data.Shader->GetRendererID(), "u_ViewProjection", vp);
		Renderer::SetUniformMat4(s_Data.Shader->GetRendererID(), "u_Transform", glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)));
	}

	void Renderer::OnUpdate(Timestep ts)
	{
		s_Data.CameraController.OnUpdate(ts);

		auto vp = s_Data.CameraController.GetCamera().GetViewProjectionMatrix();
		Renderer::SetUniformMat4(s_Data.Shader->GetRendererID(), "u_ViewProjection", vp);
		Renderer::SetUniformMat4(s_Data.Shader->GetRendererID(), "u_Transform", glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)));
	}

	void Renderer::OnEvent(Event& event)
	{
		s_Data.CameraController.OnEvent(event);
	}

	void Renderer::BeginScene(const OrthographicCamera& camera)
	{
		glUseProgram(s_Data.Shader->GetRendererID());
		s_Data.Shader->SetUniformMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
		s_Data.Shader->SetUniformMat4("u_Transform", glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)));
	}

	void Renderer::EndScene()
	{
	}

	const Renderer::Stats& Renderer::GetStats()
	{
		return s_Data.RenderStats;
	}

	void Renderer::ResetStats()
	{
		memset(&s_Data.RenderStats, 0, sizeof(Stats));
	}
	void Renderer::SetUniformMat4(uint32_t shader, const char* name, const glm::mat4& matrix)
	{
		int loc = glGetUniformLocation(shader, name);
		glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(matrix));
	}
}