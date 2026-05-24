#include "Application.h"

#include "Window/Input.h"
#include "Utils.h"
#include <thread>

Application::Application()
{
    // Window
    {
        WindowInfo info{};
        info.width = 800;
        info.height = 600;
        info.title = "FPS Game";

        m_Window = std::make_shared<Window>(info);
    }

    // Renderer
    {
        m_Renderer = std::make_shared<Renderer>(m_Window);
    }
}

Application::~Application()
{
    slogLoggerDestroy(Utils::GetLogger());
}

void Application::Run()
{
    m_Window->Show();
    while(m_Window->IsOpen())
    {
        m_Window->Clear();
        
        Render();
        
        Update();
    }
}

void Application::Render()
{
    m_Renderer->Render();
}

void Application::Update()
{
    m_Renderer->Update();
    m_Window->Update();

    if(Input::IsKeyPressed(*m_Window, GLFW_KEY_ESCAPE))
        m_Window->Close();
}