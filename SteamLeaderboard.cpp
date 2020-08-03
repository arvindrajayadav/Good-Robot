#include "master.h"
#include "SteamLeaderboard.h"

#define LEADERBOARD_SCORE "High Scores"
#define LEADERBOARD_KILLS "Robots Destroyed"

using namespace pyrodactyl;

SteamLeaderboard::SteamLeaderboard()
{
	loading = false;
	handle_score = 0;
	handle_kills = 0;
}

//-----------------------------------------------------------------------------
// Purpose: Gets handles for our leaderboards. If the leaderboards don't exist, do not create them.
// Each time this is called, we look up another leaderboard.
//-----------------------------------------------------------------------------
void SteamLeaderboard::FindLeaderboards()
{
	//We're already downloading leaderboards, wait
	if (loading)
		return;

	if (SteamUserValid())
	{
		SteamAPICall_t hSteamAPICall = 0;

		if (handle_score == 0)
		{
			// find/create a leaderboard for the quickest win
			hSteamAPICall = PlayerSteamStats()->FindLeaderboard(LEADERBOARD_SCORE);
		}
		else if (handle_kills == 0)
		{
			// find/create a leaderboard for the most feet traveled in 1 round
			hSteamAPICall = PlayerSteamStats()->FindLeaderboard(LEADERBOARD_KILLS);
		}

		if (hSteamAPICall != 0)
		{
			// set the function to call when this API call has completed
			m_SteamCallResultFindLeaderboard.Set(hSteamAPICall, this, &SteamLeaderboard::OnFindLeaderboard);
			loading = true;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called when SteamUserStats()->FindLeaderboard() returns asynchronously
//-----------------------------------------------------------------------------
void SteamLeaderboard::OnFindLeaderboard(LeaderboardFindResult_t *pFindLeaderboardResult, bool bIOFailure)
{
	loading = false;

	// see if we encountered an error during the call
	if (!pFindLeaderboardResult->m_bLeaderboardFound || bIOFailure)
		return;

	// check to see which leaderboard handle we just retrieved
	const char *pchName = SteamUserStats()->GetLeaderboardName(pFindLeaderboardResult->m_hSteamLeaderboard);
	if (strcmp(pchName, LEADERBOARD_SCORE) == 0)
		handle_score = pFindLeaderboardResult->m_hSteamLeaderboard;
	else if (strcmp(pchName, LEADERBOARD_KILLS) == 0)
		handle_kills = pFindLeaderboardResult->m_hSteamLeaderboard;

	//Look up any other leaderboards
	FindLeaderboards();

	//if the user is currently looking at a leaderboard, it might be one we didn't have a handle for yet. Update the leaderboard.
	//TODO: update display here
}

//-----------------------------------------------------------------------------
// Purpose: Called when SteamUserStats()->UploadLeaderboardScore() returns asynchronously
//-----------------------------------------------------------------------------
void SteamLeaderboard::OnUploadScore(LeaderboardScoreUploaded_t *pScoreUploadedResult, bool bIOFailure)
{
	if (!pScoreUploadedResult->m_bSuccess)
	{
		//TODO: error
	}

	if (pScoreUploadedResult->m_bScoreChanged)
	{
		//TODO: could display new high score
	}
}

void SteamLeaderboard::AddScore(int score, int kills)
{
	// if the user is valid, update the leaderboards with their score and kills
	if (SteamUserValid())
	{
		//If the user has a higher score already, this value will be thrown out

		if (handle_score != 0)
		{
			SteamAPICall_t hSteamAPICall = SteamUserStats()->UploadLeaderboardScore(handle_score, k_ELeaderboardUploadScoreMethodKeepBest, score, NULL, 0);
			m_SteamCallResultUploadScore.Set(hSteamAPICall, this, &SteamLeaderboard::OnUploadScore);
		}

		if (handle_kills != 0)
		{
			SteamAPICall_t hSteamAPICall = SteamUserStats()->UploadLeaderboardScore(handle_kills, k_ELeaderboardUploadScoreMethodKeepBest, kills, NULL, 0);
			m_SteamCallResultUploadScore.Set(hSteamAPICall, this, &SteamLeaderboard::OnUploadScore);
		}
	}
}