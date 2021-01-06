#include "GLCore.h"
#include "GameLayer.h"

using namespace GLCore;

class App : public Application
{
public:
	App()
	{
		PushLayer(new GameLayer());
	}
};

int main()
{
	std::unique_ptr<App> app = std::make_unique<App>();
	app->Run();
}