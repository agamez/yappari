#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define BUILD_NUMBER 5205
#define VERSION "2.0.5"
#define FULL_VERSION VERSION " Build " STR(BUILD_NUMBER)
