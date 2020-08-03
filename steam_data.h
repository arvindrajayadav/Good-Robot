#pragma once

#ifdef CMAKE_BUILD
#include "steam_api.h"
#endif


#if defined(__APPLE__)
#include "steam_api.h"
#endif

void SteamUserInit();
bool SteamUserValid();
ISteamUserStats* PlayerSteamStats();

//NOTE: The ach_id must be set via the Steam developer site, and it is case sensitive
//The string "0" is not a valid achievement id
void SteamGiveAchievement(const char* ach_id);
void SteamIndicateAchievementProgress(const char* ach_id, const int cur, const int max);

//NOTE: The stat_id must be set via the Steam developer site, and it is case sensitive
//The string "0" is not a valid stat id
void SteamSetStat(const char* stat_id, const int value);
void SteamSetStat(const char* stat_id, const float value);
void SteamUpdateAvgRate(const char* stat_id, const float value, const float session_length);

//Check the options menu
void OptionsCheckAch();

//Get the current user's Steam display name
const char* SteamDisplayName();

//Sync our stats with Steam - only do this during game over time
void SteamSync();