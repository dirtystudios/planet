#include "InputManager.h"
#include "Log.h"

namespace input {

    InputManager::InputManager() {
        for (int x = 0; x < (uint32_t)InputCode::COUNT; ++x) {
            actionCache.emplace_back(0);
        }
    }

    bool InputManager::ShouldShowCursor() {
        return m_showCursor;
    }

    void InputManager::AddActionMapping(std::string actionName, const InputCode& inputCode, const ActionConfig& actionConfig) {
        MappingConfig mappingConfig;
        mappingConfig.inputCode= inputCode;
        mappingConfig.actionConfig = actionConfig;
        mappingConfig.inputFromController = IsControllerInputCode(inputCode);
        actionMappings.insert(std::make_pair(actionName, mappingConfig));
    }

    void InputManager::AddAxisMapping(std::string axisName, const InputCode& inputCode, const AxisConfig& axisConfig) {
        MappingConfig mappingConfig;
        mappingConfig.inputCode = inputCode;
        mappingConfig.axisConfig = axisConfig;
        mappingConfig.inputFromController = IsControllerInputCode(inputCode);
        axisMappings.insert(std::make_pair(axisName, mappingConfig));
    }

    InputContext* InputManager::CreateNewContext(ContextPriority priority) {
        InputContext* inputContext = new InputContext;
        contextMappings.insert(std::make_pair((uint32_t)priority, inputContext));
        return inputContext;
    }

    KeyboardManager* InputManager::GetKeyboardManager() {
        return &m_keyboardManager;
    }

    bool InputManager::ShouldSendActionEvent(float newValue, float prevValue, ActionConfig* actionConfig) {
        // this is hurting my head, so this is the logic broken down
        // we only send 'held' events if flagged, otherwise we only care about changes
        if (newValue != prevValue) {
            if (newValue > 0.f) {
                return true;
            }
            else if (!actionConfig->ignoreRelease) {
                return true;
            }
        }
        else if (newValue > 0.f && !actionConfig->ignoreHeld) {
            return true;
        }
        return false;
    }

    void InputManager::ProcessInputs(const std::vector<float>& inputValues, float ms) {
        //todo: ....why is this a vector? it could easily just be an array the size of inputvalues
        std::vector<uint32_t> inputHandled;
        // show the cursor by default
        m_showCursor = true;

        // Call Keyboard handler first, this handles focused typing interactions
        for (uint32_t x = 0; x < inputValues.size(); ++x) {
            float prevValue = actionCache[x];
            float newValue = inputValues[x];
            bool handled = m_keyboardManager.HandlingKey((input::InputCode)x);
            if (handled) {
                inputHandled.emplace_back((uint32_t)x);
                m_keyboardManager.HandleKeyPress((input::InputCode)x, (int)newValue, ms);
            }
        }

        // k, this is a lot of loops, ughhh
        // loop through the contexts by priority
        for (int x = 0; x < (uint32_t)ContextPriority::COUNT; ++x) {
            auto its = contextMappings.equal_range(x);
            for (auto it = its.first; it != its.second; ++it) {
                int numBindings = it->second->GetNumContextBindings<ContextBindingType::Axis>();
                std::vector<uint32_t> tempAllowAxis;
                for (int y = 0; y < numBindings; ++y) {
                    ContextBinding* contextBinding = it->second->GetContextBinding<ContextBindingType::Axis>(y);
                    auto range = axisMappings.equal_range(contextBinding->mappingName);
                    for (auto it2 = range.first; it2 != range.second; ++it2) {
                        MappingConfig *mapConfig = &it2->second;
                        float scale = mapConfig->axisConfig.scale;
                        uint32_t inputCode = (uint32_t)mapConfig->inputCode;
                        if (std::find(inputHandled.begin(), inputHandled.end(), inputCode) == inputHandled.end() 
                            || std::find(tempAllowAxis.begin(), tempAllowAxis.end(), inputCode) != tempAllowAxis.end()) {
                            float newValue = inputValues[inputCode];
                            newValue = fabs(newValue) < mapConfig->axisConfig.deadZone ? 0 : newValue;
                            if (contextBinding->boundDelegate(InputContextCallbackArgs(newValue * scale, mapConfig->inputFromController))) {
                                inputHandled.emplace_back(inputCode);
                            }
                            tempAllowAxis.emplace_back(inputCode);
                        }
                    }
                }

                // loop through each action
                numBindings = it->second->GetNumContextBindings<ContextBindingType::Action>();
                for (int y = 0; y < numBindings; ++y) {
                    ContextBinding* contextBinding = it->second->GetContextBinding<ContextBindingType::Action>(y);

                    // is someone registered for this binding?, get all of them
                    auto range = actionMappings.equal_range(contextBinding->mappingName);
                    for (auto it2 = range.first; it2 != range.second; ++it2) {
                        MappingConfig *mapConfig = &it2->second;
                        uint32_t inputCode = (uint32_t)mapConfig->inputCode;

                        // find out what 'key/input' they are registered for and give it to them
                        // but only if it hasn't already been 'handled' by a higher priority context
                        if (std::find(inputHandled.begin(), inputHandled.end(), inputCode) == inputHandled.end()) {
                            float prevValue = actionCache[inputCode];
                            float newValue = inputValues[inputCode];
                            ActionConfig* actionConfig = &mapConfig->actionConfig;

                            // pass the key/input only if binding wants event and tag it as handled.
                            if (ShouldSendActionEvent(newValue, prevValue, actionConfig)) {
                                // did delegate 'handle' the input?
                                if (contextBinding->boundDelegate(InputContextCallbackArgs(newValue, mapConfig->inputFromController))) {
                                    inputHandled.emplace_back(inputCode);

                                    if (actionConfig->hideCursor) m_showCursor = newValue > 0 ? false : true;
                                }
                            }
                        }
                    }
                }
            }
        }
        actionCache = inputValues;
    }
}