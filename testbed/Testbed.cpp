#include "EntryPoint.h"
#include "sge.h"

#include "tests/AnimationGraphTests.h"

class Testbed : public Application
{
public:
    void start() override
    {
        logInfo("========== Testbed ==========");

        runAnimationGraphTests();

        logInfo("========== All tests PASSED ==========");

        Engine::get()->stop();
    }
};

Application* CreateApplication()
{
    return new Testbed();
}
