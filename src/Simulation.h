#pragma once

#include "SimObj.h"
#include <set>

class Simulation {
private:
    std::set<SimObj*> _simObjs;
public:
    SimObj* AddSimObj() {
        SimObj* simObj = new SimObj();
        _simObjs.insert(simObj);
        return simObj;
    }

    void RemoveSimObj(SimObj* simObj) {
        _simObjs.erase(simObj);
    }

    void Update(double delta) {
        for(SimObj* simObj : _simObjs) {
            simObj->Update(delta);
        }
    }
};