#pragma once

#include <unordered_map>
#include "InputContext.h"
#include "InputCodes.h"

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
            ActionConfig() : ignoreRelease(true) {};
            ActionConfig(bool p_ignoreRelease) : ignoreRelease(p_ignoreRelease) {};
        };

        enum class ContextPriority : uint32_t {
            CONTEXT_MENU = 0,
            CONTEXT_PLAYER,
            CONTEXT_WORLD,
            COUNT,
        };
    private:
        struct MappingConfig {
            InputCode inputCode;
            AxisConfig axisConfig;
            ActionConfig actionConfig;
        };

        std::unordered_multimap<std::string, MappingConfig> axisMappings;
        std::unordered_multimap<std::string, MappingConfig> actionMappings;
        std::unordered_multimap<uint32_t, InputContext*> contextMappings;
        std::vector<int> actionCache;

    public:
        InputManager();
        void AddActionMapping(std::string actionName, const InputCode& inputCode, const ActionConfig& actionConfig);
        void AddAxisMapping(std::string axisName, const InputCode& inputCode, const AxisConfig& axisConfig);

        InputContext* CreateNewContext(ContextPriority priority);

        void ProcessInputs(const std::vector<float>& inputValues, float dt);
    };
}