#include "platform/System.h"
#include "App.h"

#ifdef WINAPI_PARTITION_APP
// UWP wants/needs a c++/cli main as far as i can tell
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^) {
#else
int main(int argc, char** argv) {
#endif
    App app;
    sys::Run(&app);
}