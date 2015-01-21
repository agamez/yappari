#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define BUILD_NUMBER 4841
#define VERSION "1.1.54"
#define FULL_VERSION VERSION " Build " STR(BUILD_NUMBER)
