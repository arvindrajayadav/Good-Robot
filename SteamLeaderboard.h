#pragma once

#include "steam_data.h"

namespace pyrodactyl
{
	class SteamLeaderboard
	{
		//True if we're looking up a leaderboard handle
		bool loading;

		// Called when SteamUserStats()->FindLeaderboard() returns asynchronously
		void OnFindLeaderboard(LeaderboardFindResult_t *pFindLearderboardResult, bool bIOFailure);
		CCallResult<SteamLeaderboard, LeaderboardFindResult_t> m_SteamCallResultFindLeaderboard;

		// Called when SteamUserStats()->UploadLeaderboardScore() returns asynchronously
		void OnUploadScore(LeaderboardScoreUploaded_t *pFindLearderboardResult, bool bIOFailure);
		CCallResult<SteamLeaderboard, LeaderboardScoreUploaded_t> m_SteamCallResultUploadScore;

	public:

		//The handles to our leaderboards
		SteamLeaderboard_t handle_score, handle_kills;

		SteamLeaderboard();

		//Download leaderboards from Steam
		void FindLeaderboards();

		//Add a score to leaderboard
		void AddScore(int score, int kills);
	};
}