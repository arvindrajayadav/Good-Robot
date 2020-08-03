#pragma once

#include "steam_data.h"
#include "menu.h"

namespace pyrodactyl
{
	//This class stores the entries for a single Steam leaderboard
	struct SteamLeaderboardData
	{
		static const int k_nMaxLeaderboardEntries = 10;       //maximum number of leaderboard entries we can display
		LeaderboardEntry_t m_leaderboardEntries[k_nMaxLeaderboardEntries];   //leaderboard entries we received from DownloadLeaderboardEntries

		int m_nLeaderboardEntries;                      //number of leaderboard entries we received
		SteamLeaderboard_t m_hSteamLeaderboard;			//handle to the leaderboard we are displaying
		ELeaderboardDataRequest m_eLeaderboardData;		//type of data we are displaying

		bool m_bLoading;   // waiting to receive leaderboard results
		bool m_bIOFailure; // last attempt to retrieve the leaderboard failed

		CCallResult<SteamLeaderboardData, LeaderboardScoresDownloaded_t> m_callResultDownloadEntries;

	public:
		SteamLeaderboardData();

		void DownloadEntries(SteamLeaderboard_t hLeaderboard, ELeaderboardDataRequest eLeaderboardData, int offset);

		//Called when SteamUserStats()->DownloadLeaderboardEntries() returns asynchronously
		void OnLeaderboardDownloadedEntries(LeaderboardScoresDownloaded_t *pLeaderboardScoresDownloaded, bool bIOFailure);
	};
}