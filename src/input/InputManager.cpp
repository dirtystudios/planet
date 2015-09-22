
#include "InputManager.h"

namespace input {

    InputManager::InputManager() {
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
        // k, this is a lot of loops, ughhh
        for (int x = 0; x < (uint32_t)ContextPriority::COUNT; ++x) {
            auto its = contextMappings.equal_range(x);
            for (auto it = its.first; it != its.second; ++it) {
                int numBindings = it->second->InputContext::GetNumContextBindings<ContextBindingType::Axis>();
                for (int y = 0; y < numBindings; ++y) {
                    ContextBinding* contextBinding = it->second->InputContext::GetContextBinding<ContextBindingType::Axis>(y);
                    auto it2 = axisMappings.find(contextBinding->mappingName);
                    if (it2 != axisMappings.end()) {
                        float scale = it2->second.axisConfig.scale;
                        uint32_t inputCode = (uint32_t)it2->second.inputCode;
                        float newValue = inputValues[inputCode];
                        newValue = fabs(newValue) < it2->second.axisConfig.deadZone ? 0 : newValue;
                        contextBinding->boundDelegate(newValue * scale);
                    }
                }
                numBindings = it->second->InputContext::GetNumContextBindings<ContextBindingType::Action>();
                for (int y = 0; y < numBindings; ++y) {
                    ContextBinding* contextBinding = it->second->InputContext::GetContextBinding<ContextBindingType::Action>(y);
                    auto it2 = actionMappings.find(contextBinding->mappingName);
                    if (it2 != actionMappings.end()) {
                        uint32_t inputCode = (uint32_t)it2->second.inputCode;
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