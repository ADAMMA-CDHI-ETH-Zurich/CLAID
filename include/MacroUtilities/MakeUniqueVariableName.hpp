#include "Concat.hpp"

/// Make a unique variable name containing the line number at the end of the
/// name. Ex: `uint64_t MAKE_UNIQUE_VARIABLE_NAME(counter) = 0;` would
/// produce `uint64_t counter_7 = 0` if the call is on line 7!
#define MAKE_UNIQUE_VARIABLE_NAME(prefix) CONCAT(prefix##_, __COUNTER__)