#pragma once

// Mach Specific
#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <libgen.h>
#include <unistd.h>
#else
#include <unistd.h>
#include <limits.h>
#endif

// Glm stuff
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_access.hpp>

// 'Standard' things
#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <iomanip>
#include <list>
#include <map>
#include <memory>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
