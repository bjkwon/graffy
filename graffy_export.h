

#ifdef GRAFFY_INTERNAL //====================== GRAFFY_INTERNAL
#define GRAPHY_EXPORT __declspec (dllexport)
#else
#define GRAPHY_EXPORT __declspec (dllimport)
#endif                //====================== GRAFFY_INTERNAL

