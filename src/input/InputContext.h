#pragma once
//#include <srutil/delegate/delegate.hpp>
#include "Callback.h"
#include <string>
#include <vector>


namespace input {
    //typedef void(*InputContextDelegate)(float value);
    //using InputContextDelegate = srutil::delegate1<void, float>;
    typedef Callback<void(float)> InputContextCallback;

    struct ContextBinding {
        std::string mappingName;
        InputContextCallback boundDelegate;
    };

    enum class ContextBindingType {
        Action,
        Axis,
    };

    class InputContext {
    private:
        std::vector<ContextBinding> contextAxisBindings;
        std::vector<ContextBinding> contextActionBindings;
    public:
        template<ContextBindingType T>
        void BindContext(std::string name, InputContextCallback inputDelegate) {
            ContextBinding contextBinding;
            contextBinding.mappingName = name;
            contextBinding.boundDelegate = inputDelegate;

            switch (T) {
            case ContextBindingType::Action: contextActionBindings.emplace_back(contextBinding); break;
            case ContextBindingType::Axis: contextAxisBindings.emplace_back(contextBinding); break;
            }
        }

        template<ContextBindingType T>
        int GetNumContextBindings() {
            switch (T) {
            case ContextBindingType::Action: return contextActionBindings.size();
            case ContextBindingType::Axis: return contextAxisBindings.size();
            }
        }

        template<ContextBindingType T>
        ContextBinding* GetContextBinding(int index) {
            switch (T) {
            case ContextBindingType::Action: return &contextActionBindings[index];
            case ContextBindingType::Axis: return &contextAxisBindings[index];
            }
        }
    };
}