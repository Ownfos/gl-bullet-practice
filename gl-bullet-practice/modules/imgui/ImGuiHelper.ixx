module;

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>
#include <fmt/format.h>

export module ImGuiHelper;

export import Window;

using namespace ownfos::opengl;

export namespace ownfos::imgui
{
    class ImGuiHelper
    {
    public:
        ImGuiHelper(Window& window, int glsl_version = 330)
        {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImPlot::CreateContext();
            ImGui_ImplGlfw_InitForOpenGL(window.get_glfw_window(), true);
            ImGui_ImplOpenGL3_Init(fmt::format("#version {}", glsl_version).c_str());
        }

        ~ImGuiHelper()
        {
            ImPlot::DestroyContext();
            ImGui::DestroyContext();
        }

        template<typename T>
        void render(T render_function)
        {
            begin_frame();
            render_function();
            end_frame();
        }

    private:
        void begin_frame()
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
        }

        void end_frame()
        {
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
    };
} // namespace ownfos::imgui
