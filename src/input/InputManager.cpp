#include "InputManager.h"

namespace input {

    InputManager::InputManager() {
        mappedKeyboard = false;
        for (int x = 0; x < (uint32_t)InputCode::COUNT; ++x) {
            actionCache.emplace_back(0);
        }
    }

    void InputManager::AddActionMapping(std::string actionName, const InputCode& inputCode, const ActionConfig& actionConfig) {
        MappingConfig mappingConfig;
        mappingConfig.inputCode= inputCode;
        mappingConfig.actionConfig = actionConfig;
        actionMappings.insert(std::make_pair(actionName, mappingConfig));
    }

    void InputManager::AddAxisMapping(std::string axisName, const InputCode& inputCode, const AxisConfig& axisConfig) {
        MappingConfig mappingConfig;
        mappingConfig.inputCode = inputCode;
        mappingConfig.axisConfig = axisConfig;
        axisMappings.insert(std::make_pair(axisName, mappingConfig));
    }

    InputContext* InputManager::CreateNewContext(ContextPriority priority) {
        InputContext* inputContext = new InputContext;
        contextMappings.insert(std::make_pair((uint32_t)priority, inputContext));
        return inputContext;
    }

    void InputManager::ProcessInputs(const std::vector<float>& inputValues, float dt) {
        std::vector<uint32_t> inputHandled;
        // k, this is a lot of loops, ughhh
        for (int x = 0; x < (uint32_t)ContextPriority::COUNT; ++x) {
            auto its = contextMappings.equal_range(x);
            for (auto it = its.first; it != its.second; ++it) {
                int numBindings = it->second->GetNumContextBindings<ContextBindingType::Axis>();
                std::vector<uint32_t> tempAllowAxis;
                for (int y = 0; y < numBindings; ++y) {
                    ContextBinding* contextBinding = it->second->GetContextBinding<ContextBindingType::Axis>(y);
                    auto it2 = axisMappings.find(contextBinding->mappingName);
                    if (it2 != axisMappings.end()) {
                        float scale = it2->second.axisConfig.scale;
                        uint32_t inputCode = (uint32_t)it2->second.inputCode;
                        if (std::find(inputHandled.begin(), inputHandled.end(), inputCode) == inputHandled.end() 
                            || std::find(tempAllowAxis.begin(), tempAllowAxis.end(), inputCode) != tempAllowAxis.end()) {
                            float newValue = inputValues[inputCode];
                            newValue = abs(newValue) < it2->second.axisConfig.deadZone ? 0 : newValue;
                            contextBinding->boundDelegate(newValue * scale);
                            inputHandled.emplace_back(inputCode);
                            tempAllowAxis.emplace_back(inputCode);
                        }
                    }
                }
                numBindings = it->second->GetNumContextBindings<ContextBindingType::Action>();
                for (int y = 0; y < numBindings; ++y) {
                    ContextBinding* contextBinding = it->second->GetContextBinding<ContextBindingType::Action>(y);
                    auto it2 = actionMappings.find(contextBinding->mappingName);
                    if (it2 != actionMappings.end()) {
                        uint32_t inputCode = (uint32_t)it2->second.inputCode;
                        if (std::find(inputHandled.begin(), inputHandled.end(), inputCode) == inputHandled.end()) {
                            float prevValue = actionCache[inputCode];
                            float newValue = inputValues[inputCode];
                            if (newValue != prevValue
                                && ((it2->second.actionConfig.ignoreRelease && newValue == 0)
                                    || (!it2->second.actionConfig.ignoreRelease))) {
                                contextBinding->boundDelegate(newValue);
                            }
                            actionCache[inputCode] = newValue;
                        }
                    }
                }
            }
        }
    }

    void InputManager::PopulateDefaultKeyboardBindings() {
        if (mappedKeyboard)
            return;
        mappedKeyboard = true;
#define MapKeyboardKey(x) AddActionMapping("##x", InputCode::INPUT_##x, ActionConfig(false));
        MapKeyboardKey(KEY_A);
        MapKeyboardKey(KEY_B);
        MapKeyboardKey(KEY_C);
        MapKeyboardKey(KEY_D);
        MapKeyboardKey(KEY_E);
        MapKeyboardKey(KEY_F);
        MapKeyboardKey(KEY_G);
        MapKeyboardKey(KEY_H);
        MapKeyboardKey(KEY_I);
        MapKeyboardKey(KEY_J);
        MapKeyboardKey(KEY_K);
        MapKeyboardKey(KEY_L);
        MapKeyboardKey(KEY_M);
        MapKeyboardKey(KEY_N);
        MapKeyboardKey(KEY_O);
        MapKeyboardKey(KEY_P);
        MapKeyboardKey(KEY_Q);
        MapKeyboardKey(KEY_R);
        MapKeyboardKey(KEY_S);
        MapKeyboardKey(KEY_T);
        MapKeyboardKey(KEY_U);
        MapKeyboardKey(KEY_V);
        MapKeyboardKey(KEY_W);
        MapKeyboardKey(KEY_X);
        MapKeyboardKey(KEY_Y);
        MapKeyboardKey(KEY_Z);
        MapKeyboardKey(KEY_0);
        MapKeyboardKey(KEY_1);
        MapKeyboardKey(KEY_2);
        MapKeyboardKey(KEY_3);
        MapKeyboardKey(KEY_4);
        MapKeyboardKey(KEY_5);
        MapKeyboardKey(KEY_6);
        MapKeyboardKey(KEY_7);
        MapKeyboardKey(KEY_8);
        MapKeyboardKey(KEY_9);
        MapKeyboardKey(KEY_LEFT_SHIFT);
        MapKeyboardKey(KEY_RIGHT_SHIFT);
        MapKeyboardKey(KEY_TILDE);
        MapKeyboardKey(KEY_CAPSLOCK);
        MapKeyboardKey(KEY_BACKSPACE);
#undef MapKeyboardKey
    }
}