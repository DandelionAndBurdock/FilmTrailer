#include "../../nclgl/window.h"
#include "Renderer.h"

#pragma comment(lib, "nclgl.lib")

int main() {
	Window w("Scene Management!", 1280, 720, false);
	if (!w.HasInitialised()) {
		return -1;
	}

	std::vector<float> coords = { 0.0, 2.0, 4.0, 5.0, 0.0 };
	std::vector<float> time = { 0.0, 1.0, 2.0, 3.0, 4.0 };


	Renderer renderer(w);
	if(!renderer.HasInitialised()) {
		return -1;
	}

	w.LockMouseToWindow(true);
	w.ShowOSPointer(false);

	w.GetTimer()->GetTimedMS();
	while(w.UpdateWindow() && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE)){
		renderer.UpdateScene(w.GetTimer()->GetTimedMS());
		renderer.RenderScene();
	}

	return 0;
}