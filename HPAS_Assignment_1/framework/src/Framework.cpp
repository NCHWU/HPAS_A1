#include "Framework.h"

bool Framework::initialise()
{
    PlotRenderer::init();
    FontHandler::initialise();
    FontHandler::loadFont(RESOURCE_ROOT "fonts/RobotoMono-Medium.ttf", 0.7f, 0.8f, 48);
    return true;
}

void Framework::terminate()
{
    FontHandler::terminate();
}
