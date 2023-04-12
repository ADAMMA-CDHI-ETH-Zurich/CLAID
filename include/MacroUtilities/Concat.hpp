#define CONCAT_(prefix, suffix) prefix##suffix
/// Concatenate `prefix, suffix` into `prefixsuffix`
#define CONCAT(prefix, suffix) CONCAT_(prefix, suffix)

