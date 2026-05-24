#include "Renderer.h"

#include "GuiHelper.h"
#include "Window/Input.h"

#include <imgui/imgui.h>

#include <stb/stb_image.h>
#include <iostream>

Renderer::Renderer(std::shared_ptr<Window> window)
{
    m_Window = window;

    m_Model.Init("assets/meshes/car/covered_car.gltf");
    m_Skybox.Init("assets/skybox/1.hdr");

    m_Fb.Init(window->GetWindowInfo().width, window->GetWindowInfo().height);
    m_SceneShader.Init("assets/shaders/scene.glsl");
    m_FinalPassShader.Init("assets/shaders/final.glsl");

    // Fullscreen quad
    {
        glGenVertexArrays(1, &m_QuadVao);
        glGenBuffers(1, &m_QuadVbo);

        glBindVertexArray(m_QuadVao);
        glBindBuffer(GL_ARRAY_BUFFER, m_QuadVbo);
        
        float quadVertices[] = {
            // positions   // texCoords
            -1.0f,  1.0f,   0.0f, 1.0f,
            -1.0f, -1.0f,   0.0f, 0.0f,
             1.0f, -1.0f,   1.0f, 0.0f,

            -1.0f,  1.0f,   0.0f, 1.0f,
             1.0f, -1.0f,   1.0f, 0.0f,
             1.0f,  1.0f,   1.0f, 1.0f
        };

        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 6, quadVertices, GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(float) * 4, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(float) * 4, (void*)(sizeof(float) * 2));

        glBindVertexArray(0);
    }

    GuiHelper::Init(m_Window, false);

    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
}

Renderer::~Renderer()
{
    GuiHelper::Shutdown();

    glDeleteBuffers(1, &m_QuadVbo);
    glDeleteVertexArrays(1, &m_QuadVao);

    m_Skybox.Destroy();
    m_Model.Destroy();
    m_Fb.Destroy();
    m_FinalPassShader.Destroy();
    m_SceneShader.Destroy();
}

void Renderer::Render()
{
    WindowInfo winInfo = m_Window->GetWindowInfo();
    if(!winInfo.width || !winInfo.height)
        return;

    // Main Rendering
    m_Fb.Resize(winInfo.width, winInfo.height);
    m_Fb.Bind();
    glViewport(0, 0, m_Fb.GetColorAttachment().GetWidth(), m_Fb.GetColorAttachment().GetHeight());
    {
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_SceneShader.Bind();
        m_SceneShader.PutMat4("proj", m_Camera.GetProjMat());
        m_SceneShader.PutMat4("view", m_Camera.GetViewMat());
        m_SceneShader.PutVec3("cameraPos", m_Camera.GetPos());
        m_SceneShader.PutTex("u_Albedo", 0);
        
        m_Model.Render("model", m_SceneShader);
        m_Skybox.Render(m_Camera);
    }

    m_Fb.Unbind();
    glViewport(0, 0, winInfo.width, winInfo.height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Rendering the quad while resampling in the final shader
    {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        
        m_FinalPassShader.Bind();        
        m_FinalPassShader.PutTex("u_Scene", 0);
        if(Input::IsKeyPressed(*m_Window, GLFW_KEY_G))
            m_FinalPassShader.PutInt("u_ShowGrey", 1);
        else
            m_FinalPassShader.PutInt("u_ShowGrey", 0);
        
        m_Fb.GetColorAttachment().Active(1);
        m_Fb.GetColorAttachment().Bind();

        glBindVertexArray(m_QuadVao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        m_FinalPassShader.Unbind();
    }
 
    // ImGui rendering
    GuiHelper::StartFrame();

    ImGui::Begin("Info");

    ImGui::Text("Delta Time :- %0.2fms", m_Delta * 1000);
    ImGui::Text("%.0f FPS", 1/m_Delta);
    ImGui::Text("Press 'G' for grey scaled output!!");

    ImGui::End();

    GuiHelper::EndFrame();
    GuiHelper::Update(m_Window);
}

void Renderer::Update()
{
    float crntTime = (float)glfwGetTime();
    m_Delta = crntTime - m_LastTime;
    m_LastTime = crntTime;

    if(m_Window->GetWindowInfo().width > 0 && m_Window->GetWindowInfo().height > 0)
        m_Camera.Update(*m_Window, (float)m_Window->GetWindowInfo().width/(float)m_Window->GetWindowInfo().height, m_Delta);
}
