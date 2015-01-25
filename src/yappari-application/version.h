#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define BUILD_NUMBER 5202
#define VERSION "2.0.2"
#define FULL_VERSION VERSION " Build " STR(BUILD_NUMBER)
