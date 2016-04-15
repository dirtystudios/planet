#pragma once
#include <Windows.h>
#include <agile.h>
#include "InputUWP.h"

// This is just 'wrapper' class to handle the c++/cli needed boilerplate

ref class PlanetApplicationSource sealed : Windows::ApplicationModel::Core::IFrameworkViewSource
{
public:
    virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView();
};

// This is mostly a copy paste from universal windows examples simple3dgamedx

ref class PlanetUWPApp sealed : public Windows::ApplicationModel::Core::IFrameworkView
{
private:
    // Cached reference to the Window.
    Platform::Agile<Windows::UI::Core::CoreWindow>  m_window;
    float m_dpi;
    std::vector<float> m_inputValues;

    InputUWP^ m_input;

public:
    // unused
    PlanetUWPApp() {};

    // IFrameworkView Methods
    virtual void Initialize(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView);
    virtual void SetWindow(Windows::UI::Core::CoreWindow^ window);
    virtual void Load(Platform::String^ entryPoint);
    virtual void Run();
    // ignoring this one for now
    virtual void Uninitialize() {};

protected:
    // Application lifecycle event handlers.
    void OnActivated(
        Windows::ApplicationModel::Core::CoreApplicationView^ applicationView,
        Windows::ApplicationModel::Activation::IActivatedEventArgs^ args
    );

    /* unknown what to do with these yet
    void OnSuspending(
        Platform::Object^ sender,
        Windows::ApplicationModel::SuspendingEventArgs^ args
    );
    void OnResuming(
        Platform::Object^ sender,
        Platform::Object^ args
    );
    */

    // Window event handlers.
    void OnWindowSizeChanged(
        Windows::UI::Core::CoreWindow^ sender,
        Windows::UI::Core::WindowSizeChangedEventArgs^ args
    );

    /* unknown
    void OnVisibilityChanged(
        Windows::UI::Core::CoreWindow^ sender,
        Windows::UI::Core::VisibilityChangedEventArgs^ args
    );
    void OnWindowClosed(
        Windows::UI::Core::CoreWindow^ sender,
        Windows::UI::Core::CoreWindowEventArgs^ args
    );
    void OnWindowActivationChanged(
        Windows::UI::Core::CoreWindow^ sender,
        Windows::UI::Core::WindowActivatedEventArgs^ args
    );*/

    // DisplayInformation event handlers.
    void OnDpiChanged(
        Windows::Graphics::Display::DisplayInformation^ sender,
        Platform::Object^ args
    );
    /** --ignoring for now
    void OnOrientationChanged(
        Windows::Graphics::Display::DisplayInformation^ sender,
        Platform::Object^ args
    );
    void OnDisplayContentsInvalidated(Windows::Graphics::Display::DisplayInformation^ sender,
        Platform::Object^ args
    );
    void OnStereoEnabledChanged(
        Windows::Graphics::Display::DisplayInformation^ sender,
        Platform::Object^ args
    );
    **/
};