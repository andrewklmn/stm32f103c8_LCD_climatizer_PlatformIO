#define LIVING_ROOM 1
#define BEDROOM 2
#define YELLOW_BEDROOM 3
//================ here you can set the current location of device =============
#define LOCATION BEDROOM
//==============================================================================

#if LOCATION==LIVING_ROOM
  #define SELF_HEATING_TEMP_DELTA 1
#elif LOCATION==YELLOW_BEDROOM
  #define SELF_HEATING_TEMP_DELTA 4
#else
  // ====== BEDROOM
  #define SELF_HEATING_TEMP_DELTA 1
#endif

