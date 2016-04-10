#pragma once

#include <stdint.h>

class IdObj {
private:
	uint64_t _id;
public:
	IdObj() {
		static uint64_t key = 0;
		_id = ++key; 
	}

	uint64_t GetId() const {
		return _id;
	}
};