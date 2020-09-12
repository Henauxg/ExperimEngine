#include "Application.hpp"

#include <iostream>

#include <test/Quicktest.hpp>

/* File private constants */
namespace {

const char* APPLICATION_NAME = "ExperimEngineApplication";

} // namespace

int main(int argc, char* argv[])
{
	try
	{
		Application app;
		app.run();
	} catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

Application::Application()
{
	engine_ = std::make_unique<expengine::Engine>(APPLICATION_NAME);
}

void Application::run()
{
	expengine::quicktest::testSol();
	expengine::quicktest::testLuaJit();

	engine_->run();
}
