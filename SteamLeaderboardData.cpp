#include "master.h"
#include "SteamLeaderboardData.h"

using namespace pyrodactyl;

SteamLeaderboardData::SteamLeaderboardData()
{
	m_hSteamLeaderboard = 0;
	m_nLeaderboardEntries = 0;
	m_bLoading = false;
	m_bIOFailure = false;
}

void SteamLeaderboardData::DownloadEntries(SteamLeaderboard_t hLeaderboard, ELeaderboardDataRequest eLeaderboardData, int offset)
{
	m_hSteamLeaderboard = hLeaderboard;
	m_eLeaderboardData = eLeaderboardData;
	m_bLoading = true;
	m_bIOFailure = false;

	if (hLeaderboard != 0 && SteamUserValid())
	{
		// load the specified leaderboard data. We only display k_nMaxLeaderboardEntries entries at a time
		SteamAPICall_t hSteamAPICall = PlayerSteamStats()->DownloadLeaderboardEntries(hLeaderboard, eLeaderboardData,
			offset, offset + k_nMaxLeaderboardEntries);

		// Register for the async callback
		m_callResultDownloadEntries.Set(hSteamAPICall, this, &SteamLeaderboardData::OnLeaderboardDownloadedEntries);
	}
}

void SteamLeaderboardData::OnLeaderboardDownloadedEntries(LeaderboardScoresDownloaded_t *pLeaderboardScoresDownloaded, bool bIOFailure)
{
	m_bLoading = false;
	m_bIOFailure = bIOFailure;

	// leaderboard entries handle will be invalid once we return from this function. Copy all data now.
	m_nLeaderboardEntries = MIN(pLeaderboardScoresDownloaded->m_cEntryCount, k_nMaxLeaderboardEntries);
	for (int index = 0; index < m_nLeaderboardEntries; index++)
	{
		PlayerSteamStats()->GetDownloadedLeaderboardEntry(pLeaderboardScoresDownloaded->m_hSteamLeaderboardEntries, index, &m_leaderboardEntries[index], NULL, 0);
	}
}