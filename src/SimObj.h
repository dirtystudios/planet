#pragma once

#include "IdObj.h"
#include "ComponentType.h"
#include "Component.h"
#include <vector>
#include <unordered_map>

class SimObj : public IdObj {
private:
	std::unordered_map<ComponentType, Component*> _componentMap;
public:
	bool HasComponents(const std::vector<ComponentType>& components) {
		for(const ComponentType& type : components) {
			if(_componentMap.find(type) == _componentMap.end()) {
				return false;
			}
		}
		return true;
	}

	template <typename T> 
	T* GetComponent(ComponentType type) {
		auto compIt = _componentMap.find(type);
		return compIt == _componentMap.end() ? nullptr : (T*)(compIt->second);
	}	

	template <typename T> 
	T* AddComponent(ComponentType type) {
		T* comp = GetComponent<T>(type);
		if(comp == nullptr) {
			comp = new T();
			_componentMap.insert(std::make_pair(type, comp));
		}
		return comp;
	}	

	virtual void Update(double dt) { };
};
