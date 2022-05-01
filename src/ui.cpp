#include "utils.h"
#include "OpenGL.h"
#include "imgui.h"
#include "examples/imgui_impl_opengl2.h"
#include "atto/app.h"
#include <math.h>

#define FRAMES 512
static struct {
	float frame_times[FRAMES] = {};
	unsigned frame_times_cursor = 0;
} g;

void ui_init() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.BackendPlatformName = "atto";
	io.KeyMap[ImGuiKey_Tab] = AK_Tab;
	io.KeyMap[ImGuiKey_LeftArrow] = AK_Left;
	io.KeyMap[ImGuiKey_RightArrow] = AK_Right;
	io.KeyMap[ImGuiKey_UpArrow] = AK_Up;
	io.KeyMap[ImGuiKey_DownArrow] = AK_Down;
	io.KeyMap[ImGuiKey_PageUp] = AK_PageUp;
	io.KeyMap[ImGuiKey_PageDown] = AK_PageDown;
	io.KeyMap[ImGuiKey_Home] = AK_Home;
	io.KeyMap[ImGuiKey_End] = AK_End;
	io.KeyMap[ImGuiKey_Insert] = AK_Ins;
	io.KeyMap[ImGuiKey_Delete] = AK_Del;
	io.KeyMap[ImGuiKey_Backspace] = AK_Backspace;
	io.KeyMap[ImGuiKey_Space] = AK_Space;
	io.KeyMap[ImGuiKey_Enter] = AK_Enter;
	io.KeyMap[ImGuiKey_Escape] = AK_Esc;
	// TODO io.KeyMap[ImGuiKey_KeyPadEnter] = AK_KeyPadEnter;
	io.KeyMap[ImGuiKey_A] = AK_A;
	io.KeyMap[ImGuiKey_C] = AK_C;
	io.KeyMap[ImGuiKey_V] = AK_V;
	io.KeyMap[ImGuiKey_X] = AK_X;
	io.KeyMap[ImGuiKey_Y] = AK_Y;
	io.KeyMap[ImGuiKey_Z] = AK_Z;

	ImGui::StyleColorsDark();
	ImGui_ImplOpenGL2_Init();
}

void ui_resize() {
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2((float)a_app_state->width, (float)a_app_state->height);
	io.DisplayFramebufferScale = ImVec2(1.f, 1.f);
}

void ui_mouse() {
	const unsigned int btn = a_app_state->pointer.buttons;
	const int x = a_app_state->pointer.x;
	const int y = a_app_state->pointer.y;
	ImGuiIO& io = ImGui::GetIO();
	io.MousePos = ImVec2((float)x, (float)y);
	io.MouseDown[0] = !!(btn & AB_Left);
	io.MouseDown[1] = !!(btn & AB_Right);
	io.MouseDown[2] = !!(btn & AB_Middle);
}

void ui_key(int key /*AKey*/, int down) {
	ImGuiIO& io = ImGui::GetIO();
	IM_ASSERT(key >= 0);
	IM_ASSERT(key < sizeof(io.KeysDown));
	io.KeysDown[key] = !!down;

	io.KeyCtrl = io.KeysDown[AK_LeftCtrl] || io.KeysDown[AK_RightCtrl];
	io.KeyShift = io.KeysDown[AK_LeftShift] || io.KeysDown[AK_RightShift];
	io.KeyAlt = io.KeysDown[AK_LeftAlt] || io.KeysDown[AK_RightAlt];
	io.KeySuper = io.KeysDown[AK_LeftSuper] || io.KeysDown[AK_RightSuper];

	if (down && key >= 32 && key < 128)
		io.AddInputCharacter(key);
}

void ui_begin(float dt, float row, float sec) {
	g.frame_times[g.frame_times_cursor] = dt * 1000.f;
	g.frame_times_cursor = (g.frame_times_cursor + 1) % COUNTOF(g.frame_times);
	// if (ImGui::Button("Save"))
	// 			MySaveFunction();
	// ImGui::InputText("string", buf, IM_ARRAYSIZE(buf));
	// ImGui::SliderFloat("float", &f, 0.0f, 1.0f);

	{
		ImGui_ImplOpenGL2_NewFrame();
		ImGuiIO& io = ImGui::GetIO();
		io.DeltaTime = dt;
		IM_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built! It is generally built by the renderer back-end. Missing call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");

		ImGui::NewFrame();
	}

	//ImGui::Text("Hello, world %d", 123);
	bool show_demo_window = true;
	ImGui::ShowDemoWindow(&show_demo_window);

	ImGui::PlotLines("frame time", g.frame_times, COUNTOF(g.frame_times), g.frame_times_cursor, NULL, 0, FLT_MAX, ImVec2(0, 100));
	ImGui::LabelText("", "dt: %.3fms", 1000.f * dt);
	ImGui::LabelText("", "fps: %.3f", 1.f / dt);
	ImGui::LabelText("", "row: %d", (int)row);
	ImGui::LabelText("", "t: %02d:%06.3f", (int)(sec/60), fmodf(sec,60.f));
}

void ui_end() {
	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	ImGui::Render();
	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
}
