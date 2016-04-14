#include "System.h"

#include "Log.h"
#include "Config.h"

#include "PlatformUWP.h"
#include "DX11RenderDevice.h"

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Devices::Input;
using namespace Windows::Graphics::Display;
using namespace Windows::Foundation;
using namespace Windows::System;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::UI::ViewManagement;

// 'global' shit, bleh
PlanetUWPApp^ g_planetUwpApp = nullptr;
app::Application* g_app = { nullptr };
sys::SysWindowSize g_windowSize;
double g_counterFreq = 0.0;
LARGE_INTEGER g_counterStart;
std::vector<float> g_inputValues((int)input::InputCode::COUNT);

// 'Loader' wrapping
IFrameworkView^ PlanetApplicationSource::CreateView() {
    PlanetUWPApp^ appref = ref new PlanetUWPApp();
    g_planetUwpApp = appref;
    return g_planetUwpApp;
}

// Converts a length in device-independent pixels (DIPs) to a length in physical pixels.
inline float ConvertDipsToPixels(float dips, float dpi)
{
    static const float dipsPerInch = 96.0f;
    return floorf(dips * dpi / dipsPerInch + 0.5f); // Round to nearest integer.
}

// system.h functions
int sys::Run(app::Application* app) {
    g_app = app;
    g_windowSize.height = 600;
    g_windowSize.width = 800;
    auto directXApplicationSource = ref new PlanetApplicationSource();
    CoreApplication::Run(directXApplicationSource);
    return 0;
}

void sys::SetWindowTitle(const char* title) {
    ApplicationView^ appView = ApplicationView::GetForCurrentView();
    // stupid conversions
    std::string s_str = std::string(title);
    std::wstring wid_str = std::wstring(s_str.begin(), s_str.end());
    const wchar_t* w_char = wid_str.c_str();
    appView->Title = ref new Platform::String(w_char);
}

sys::SysWindowSize sys::GetWindowSize() {
    return g_windowSize;

}

float sys::GetTime() {
    LARGE_INTEGER li;
    if (!QueryPerformanceCounter(&li))
        LOG_E("Sys::GetTime QueryPerformanceCounter failed! -- %d", GetLastError());
    double tempTime = static_cast<double>(li.QuadPart - g_counterStart.QuadPart) / g_counterFreq;
    return static_cast<float>(tempTime);
}

void sys::ShowCursor(bool showCursor) {
    auto window = CoreWindow::GetForCurrentThread();
    if (window)
    {
        // Protect case where there isn't a window associated with the current thread.
        // This happens on initialization or when being called from a background thread.
        if (showCursor)
            window->PointerCursor = ref new CoreCursor(CoreCursorType::Arrow, 0);
        else 
            window->PointerCursor = nullptr;
    }
}

// UWP functions

void PlanetUWPApp::Run() {
    g_app->OnStart();

    float dt = 0;
    while (true) {
        float start = sys::GetTime();
        CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
        g_app->renderDevice->ResizeWindow(g_windowSize.width, g_windowSize.height);
        g_app->OnFrame(g_inputValues, dt);
        g_app->renderDevice->SwapBuffers();
        dt = sys::GetTime() - start;
    }
}

void PlanetUWPApp::Load(Platform::String^ entryPoint) {
    // unclear all that we can do here,
    // theres a chance we can *create* d3device, but current initialize may do too much

    LARGE_INTEGER li;
    if (!QueryPerformanceFrequency(&li))
        LOG_E("QueryPerformanceFreqError! -- %d", GetLastError());

    // this should give us counter in microseconds
    g_counterFreq = static_cast<double>(li.QuadPart);

    if (!QueryPerformanceCounter(&g_counterStart))
        LOG_E("QueryPerformanceCounterError! -- %d", GetLastError());

    g_app->renderDevice = new graphics::RenderDeviceDX11();
    
    graphics::DeviceInitialization devInit;
    
    devInit.windowHandle = reinterpret_cast<IUnknown*>(m_window);
    devInit.windowHeight = g_windowSize.height;
    devInit.windowWidth = g_windowSize.width;
    // *has* to be prebuilt for uwp
    devInit.usePrebuiltShaders = true; 
    g_app->renderDevice->InitializeDevice(devInit);

    g_app->renderDevice->PrintDisplayAdapterInfo();
}

// For what its worth, this seems to be called before 'load' so we will have a window by the time load hits
void PlanetUWPApp::SetWindow(Windows::UI::Core::CoreWindow^ window) {
    m_window = window;
    DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();
    m_dpi = currentDisplayInformation->LogicalDpi;


    g_windowSize.height = static_cast<uint32_t>(ConvertDipsToPixels(window->Bounds.Height, m_dpi));
    g_windowSize.width = static_cast<uint32_t>(ConvertDipsToPixels(window->Bounds.Width, m_dpi));

    window->PointerCursor = ref new CoreCursor(CoreCursorType::Arrow, 0);

    window->SizeChanged +=
        ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &PlanetUWPApp::OnWindowSizeChanged);
}

void PlanetUWPApp::OnWindowSizeChanged(CoreWindow^ window, WindowSizeChangedEventArgs^  args) {
    g_windowSize.height = static_cast<uint32_t>(args->Size.Height);
    g_windowSize.width = static_cast<uint32_t>(args->Size.Width);
}

void PlanetUWPApp::OnGamepadAdded(Platform::Object ^sender, Windows::Gaming::Input::Gamepad ^gamepad) {
}

void PlanetUWPApp::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args) {
    // This seems like all we need to do here
    CoreWindow::GetForCurrentThread()->Activate();
}

// events and 'device resources' seem like they can go here
void PlanetUWPApp::Initialize(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView) {
    applicationView->Activated +=
        ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &PlanetUWPApp::OnActivated);

    // todo, suspending, exiting, and resuming

    Windows::Gaming::Input::Gamepad::GamepadAdded +=
        ref new EventHandler<Windows::Gaming::Input::Gamepad^>(this, &PlanetUWPApp::OnGamepadAdded);
}