#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define BUILD_NUMBER 5224
#define VERSION "2.0.23"
#define FULL_VERSION VERSION " Build " STR(BUILD_NUMBER)
