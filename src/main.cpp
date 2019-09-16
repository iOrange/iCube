#include "iCubeApp.h"

int main(int, char**) {
    iCubeApp app;
    if (app.Initialize()) {
        app.Loop();
        app.Shutdown();
    }

    return 0;
}
