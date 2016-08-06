#pragma once

#include <cassert>

#define dg_assert(cond, message) assert(cond&& message)
#define dg_assert_fail(message) assert(false && message)
#define dg_assert_nm(cond) assert(cond)
#define dg_assert_fail_nm() assert(false)
#define dg_assert_not(cond, message) assert(!cond && message)
#define dg_assert_not_nm(cond) assert(!cond)
