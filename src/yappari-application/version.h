#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define BUILD_NUMBER 5216
#define VERSION "2.0.15"
#define FULL_VERSION VERSION " Build " STR(BUILD_NUMBER)
