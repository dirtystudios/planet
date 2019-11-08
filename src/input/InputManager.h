#pragma once
#include <unordered_map>
#include <memory>
#include "InputContext.h"
#include "InputCodes.h"
#include "KeyboardManager.h"

/*
----------------------
    Input Manager
----------------------
    Expected use - 
        Mappings - 
            Action - Basically a 'button' type action, will only be called on press, and called based on options
            Axis   - Constantly reports values from input key/joystick

        Add{Axis/Action}Mapping - This will register an inputcode to the defined 'stringAction'

        CreateContext - Create a context for a specific priority, mostly to figure out who gets the button first in case of multiple
        Context->BindContext
            From here you bind any stringAction, to your chosen callback
        ContextCallback 
            This will get called with the specified args, return 'false' if you would like to stop process of the 'key', basically if your action/axis should be 'blocking'
    Debug Use - 
        Debug has the highest priority
        Debug->GetDebugContext, duh
        Debug->BindContext,
            The 'stringAction' is named debug with the InputCode num appended to the string, rest is the same as above
*/

namespace input {
    class InputManager {
    public:
        struct AxisConfig {
            float scale;
            float deadZone;
            AxisConfig() : scale(0.0f), deadZone(0.0f) {};
            AxisConfig(float p_scale, float p_deadZone) : scale(p_scale), deadZone(p_deadZone) {};
        };

        struct ActionConfig {
            bool ignoreRelease;
            bool hideCursor;
            bool ignoreHeld;
            ActionConfig() : ignoreRelease(true), hideCursor(false), ignoreHeld(true) {};
            ActionConfig(bool p_ignoreRelease, bool p_ignoreHeld, bool hideCursorDuringHandled) 
                : ignoreRelease(p_ignoreRelease), hideCursor(hideCursorDuringHandled), ignoreHeld(p_ignoreHeld) {};
        };

        enum class ContextPriority : uint32_t {
            CONTEXT_DEBUG = 0,
            CONTEXT_MENU,
            CONTEXT_PLAYER,
            CONTEXT_WORLD,
            COUNT,
        };
    private:
        struct MappingConfig {
            InputCode inputCode;
            bool inputFromController;
            AxisConfig axisConfig;
            ActionConfig actionConfig;
        };

        std::unordered_multimap<std::string, MappingConfig> axisMappings;
        std::unordered_multimap<std::string, MappingConfig> actionMappings;
        std::unordered_multimap<uint32_t, std::unique_ptr<InputContext>> contextMappings;

        std::vector<float> actionCache;
        KeyboardManager m_keyboardManager;
        bool m_showCursor = true;

    public:
        InputManager();

        void AddActionMapping(std::string actionName, const InputCode& inputCode, const ActionConfig& actionConfig);
        void AddAxisMapping(std::string axisName, const InputCode& inputCode, const AxisConfig& axisConfig);

        InputContext* CreateNewContext(ContextPriority priority);
        InputContext* GetDebugContext();
        KeyboardManager* GetKeyboardManager();
        bool ShouldShowCursor();

        void ProcessInputs(const std::vector<float>& inputValues, float ms);

    private:
        bool ShouldSendActionEvent(float newValue, float prevValue, const ActionConfig& actionConfig);
        inline bool IsControllerInputCode(InputCode inputCode) {
            return (inputCode >= InputCode::INPUT_GAMEPAD_Y && inputCode <= InputCode::INPUT_GAMEPAD_GUIDE);
        }
    };
}
