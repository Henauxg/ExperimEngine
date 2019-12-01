#pragma once

namespace experimengine::render {
	class Window
	{
	public:
		Window(int width, int height, const char* title);
		~Window();

		bool shouldClose() const;
		void pollEvents();
		void waitEvents() const;
	private:

	};
}
