#define MAX_TEAMS 10
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>

// structure for last match
typedef struct
{
    char lastVenue[50];
    int lastMatchWasAway;
    int lastMatchDay;
    int lastMatchMonth;
} LastMatch;

// Structure for team details
typedef struct
{
    char name[50];
    char homeground[50];
    int homematch_count;
    int awaymatch_count;
    int afternoon_match_count;
    int evening_match_count;
} Team;

// Structure for match details
typedef struct
{
    char team1[50];
    char team2[50];
    char venue[50];
    int day;
    int month;
    int year;
    char time[10];
    int team1_runs;
    int team2_runs;
    float team1_overs;
    float team2_overs;
    int winner;
} Match;

// Structure for points table
typedef struct
{
    char team[50];
    int wins;
    int losses;
    int points;
    int ties;
    int matchesPlayed;
    int totalRunsScored;
    float totalOversFaced;
    int totalRunsConceded;
    float totalOversBowled;
    float netRunRate;
} PointsTable;
#define MAX_UNAVAILABLE_DATES 20

// structure for venue
typedef struct
{
    char venue[50];
    int unavailableDates[MAX_UNAVAILABLE_DATES][2];
    int unavailableCount;
} VenueAvailability;

Team teams[MAX_TEAMS]={{"CSK", "M. A. Chidambaram Stadium"},
{"MI", "Wankhede Stadium"},
{"RCB", "M. Chinnaswamy Stadium"},
{"KKR", "Eden Gardens"},
{"SRH", "Rajiv Gandhi International "},
{"DC", "Arun Jaitley Stadium"},
{"RR", "Sawai Mansingh Stadium"},
{"PBKS", "PCA IS Bindra Stadium"},
{"LSG", "Atal Bihari Vajpayee Ekana"},
{"GT", "Narendra Modi Stadium"}};

int start_day, start_month, end_day, end_month, start_year;
int matchIndex = 0;
Match matchDetails[100];
int teamCount = 0;
PointsTable pointsTable[MAX_TEAMS];
VenueAvailability venueAvailability[MAX_TEAMS];
LastMatch lastmatch[MAX_TEAMS];

void inputTeamsAndGrounds()
{
    printf("\nEnter Team Details:\n");
    for (int i = 0; i < MAX_TEAMS; i++)
    {
        printf("Team %d Name: ", i + 1);
        scanf(" %[^\n]s", teams[i].name);

        printf("Team %d Home Ground: ", i + 1);
        scanf(" %[^\n]s", teams[i].homeground);

        // Initialize match counts
        teams[i].homematch_count = 0;
        teams[i].awaymatch_count = 0;
        printf("\n");
    }
}

// function to find leap year
int isLeapYear(int year)
{
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
        return 1;
    return 0;
}

// Function to get the number of days in a given month
int getDaysInMonth(int month, int year)
{
    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (month == 2 && isLeapYear(year))
        return 29;

    return daysInMonth[month - 1];
}

// Function to take IPL start and end dates
void inputDates()
{
    printf("Enter the IPL start date (day month year): ");
    scanf("%d %d %d", &start_day, &start_month, &start_year);

    printf("Enter the IPL end date (day month year): ");
    scanf("%d %d %d", &end_day, &end_month, &start_year);
}

// function to get dayOfYear
int dayOfYear(int d, int m, int y)
{
    int days = d;
    for (int i = 1; i < m; ++i)
        days += getDaysInMonth(i, y);
    return days;
}

// function to get daysBetween
int daysBetween(int d1, int m1, int y1, int d2, int m2, int y2)
{
    if (y1 == y2)
        return dayOfYear(d1, m1, y1) - dayOfYear(d2, m2, y2);
    if (y1 < y2 || (y1 == y2 && m1 < m2) || (y1 == y2 && m1 == m2 && d1 < d2))
        return -daysBetween(d2, m2, y2, d1, m1, y1);

    int days = 0;
    int totalInY2 = isLeapYear(y2) ? 366 : 365;
    days += totalInY2 - dayOfYear(d2, m2, y2);
    days += dayOfYear(d1, m1, y1);
    return abs(days);
}

// take input of unvailable dates for the venues
void inputVenueAvailability()
{
    for (int i = 0; i < MAX_TEAMS; i++)
    {
        printf("Enter the venue name: %s\n", teams[i].homeground);
        strcpy(venueAvailability[i].venue, teams[i].homeground);

        printf("Enter the number of unavailable dates for %s: ", teams[i].homeground);
        scanf("%d", &venueAvailability[i].unavailableCount);

        for (int j = 0; j < venueAvailability[i].unavailableCount; j++)
        {
            printf("Enter unavailable date %d (day month): ", j + 1);
            scanf("%d %d", &venueAvailability[i].unavailableDates[j][0], &venueAvailability[i].unavailableDates[j][1]);
        }
    }
}

// constraint checker to check if the venue is available
int isDateUnavailable(char *venue, int day, int month)
{
    for (int i = 0; i < MAX_TEAMS; i++)
    {
        if (strcmp(venueAvailability[i].venue, venue) == 0)
        {
            for (int j = 0; j < venueAvailability[i].unavailableCount; j++)
            {
                if (venueAvailability[i].unavailableDates[j][0] == day &&
                    venueAvailability[i].unavailableDates[j][1] == month)
                {
                    return 1;
                }
            }
        }
    }
    return 0;
}

// Function to check if a given date falls on a weekend

int isWeekend(int day, int month, int year)
{
    struct tm date = {0};
    date.tm_year = year - 1900;
    date.tm_mon = month - 1;
    date.tm_mday = day;

    mktime(&date);

    int weekDay = date.tm_wday;

    return (weekDay == 0 || weekDay == 6);
}

// function to increment date
void incrementDate(int *day, int *month, int year)
{

    (*day)++;

    int daysInCurrentMonth = getDaysInMonth(*month, year);

    if (*day > daysInCurrentMonth)
    {
        *day = 1;
        (*month)++;
        if (*month > 12)
        {
            *month = 1;
        }
    }
}

// function to schedule the match after generation
void scheduleMatch(int homeIndex, int awayIndex, int day, int month, int year, int matchCount)
{
    strcpy(matchDetails[matchIndex].team1, teams[homeIndex].name);
    strcpy(matchDetails[matchIndex].team2, teams[awayIndex].name);
    strcpy(matchDetails[matchIndex].venue, teams[homeIndex].homeground);
    matchDetails[matchIndex].day = day;
    matchDetails[matchIndex].month = month;
    matchDetails[matchIndex].year = year;
    if (isWeekend(day, month, start_year))
    {
        if (matchCount % 2 == 0)
        {
            strcpy(matchDetails[matchIndex].time, "03:30 PM");
            teams[homeIndex].afternoon_match_count++;
            teams[awayIndex].afternoon_match_count++;
        }
        else
        {
            strcpy(matchDetails[matchIndex].time, "07:30 PM");
            teams[homeIndex].evening_match_count++;
            teams[awayIndex].evening_match_count++;
        }
    }
    else
    {
        strcpy(matchDetails[matchIndex].time, "07:30 PM");
        teams[homeIndex].evening_match_count++;
        teams[awayIndex].evening_match_count++;
    }

    strcpy(lastmatch[homeIndex].lastVenue, teams[homeIndex].homeground);
    strcpy(lastmatch[awayIndex].lastVenue, teams[homeIndex].homeground);
    lastmatch[homeIndex].lastMatchDay = day;
    lastmatch[homeIndex].lastMatchMonth = month;
    lastmatch[homeIndex].lastMatchDay = day;
    lastmatch[homeIndex].lastMatchMonth = month;

    matchIndex++;
    teams[homeIndex].homematch_count++;
    teams[awayIndex].awaymatch_count++;
}

// function to check if the team has recently played away
int hasRecentlyPlayedAway(char *team, int current_day, int current_month, int current_year)
{
    for (int i = matchIndex - 1; i >= 0; i--)
    {
        if (strcmp(matchDetails[i].team2, team) == 0)
        {
            int matchDay = matchDetails[i].day;
            int matchMonth = matchDetails[i].month;
            int matchYear = matchDetails[i].year;

            if (daysBetween(current_day, current_month, current_year, matchDay, matchMonth, matchYear) < 2)
                return 1;
            else
                break;
        }
        else if (strcmp(matchDetails[i].team1, team) == 0)
        {
            break;
        }
    }
    return 0;
}

// function to check if the day has sufficient matches
int isDayFull(int day, int month, int year, int matchCountToday)
{
    return (isWeekend(day, month, year) && matchCountToday >= 2) || (!isWeekend(day, month, year) && matchCountToday >= 1);
}

// main match scheduling algorithm
void matchScheduling()
{
    int current_day = start_day, current_month = start_month, current_year = start_year;
    int matchCountToday = 0, scheduledMatchesCount = 0;
    const int totalMatchesTarget = 70;
    const int targetHomeGames = 7;
    const int targetAwayGames = 7;

    // Initialize tracking arrays AND team home/away counts
    for (int i = 0; i < MAX_TEAMS; i++)
    {
        strcpy(lastmatch[i].lastVenue, "");
        lastmatch[i].lastMatchWasAway = 0;
        lastmatch[i].lastMatchDay = -1;
        lastmatch[i].lastMatchMonth = -1;
        teams[i].homematch_count = 0;
        teams[i].awaymatch_count = 0;
        teams[i].afternoon_match_count = 0;
        teams[i].evening_match_count = 0;
        matchDetails[i].winner = 0;
    }

    matchIndex = 0;

    // Phase 1: Round-Robin
    for (int round = 0; round < MAX_TEAMS - 1; round++)
    {
        for (int i = 0; i < MAX_TEAMS / 2; i++)
        {
            int team1_idx, team2_idx;
            int defaultHomeIdx, defaultAwayIdx;
            if (i == 0)
            {
                int fixed = MAX_TEAMS - 1, rot = round;
                defaultHomeIdx = (round % 2 != 0) ? fixed : rot;
                defaultAwayIdx = (round % 2 != 0) ? rot : fixed;
            }
            else
            {
                team1_idx = (round + i) % (MAX_TEAMS - 1);
                team2_idx = (round - i + MAX_TEAMS - 1) % (MAX_TEAMS - 1);
                defaultHomeIdx = (i % 2 == 0) ? team1_idx : team2_idx;
                defaultAwayIdx = (i % 2 == 0) ? team2_idx : team1_idx;
            }

            // Apply strict balancing
            int finalHomeIdx = -1, finalAwayIdx = -1;
            int homeCountH = teams[defaultHomeIdx].homematch_count;
            int awayCountH = teams[defaultHomeIdx].awaymatch_count;
            int homeCountA = teams[defaultAwayIdx].homematch_count;
            int awayCountA = teams[defaultAwayIdx].awaymatch_count;
            int canHomeHost = (homeCountH < targetHomeGames);
            int canAwayHost = (homeCountA < targetHomeGames);
            int mustHomeHost = (awayCountH >= targetAwayGames);
            int mustAwayHost = (awayCountA >= targetAwayGames);

            if (canHomeHost && !canAwayHost)
            {
                finalHomeIdx = defaultHomeIdx;
                finalAwayIdx = defaultAwayIdx;
            }
            else if (!canHomeHost && canAwayHost)
            {
                finalHomeIdx = defaultAwayIdx;
                finalAwayIdx = defaultHomeIdx;
            }
            else if (!canHomeHost && !canAwayHost)
            {
                continue;
            }
            else
            {
                if (mustHomeHost && !mustAwayHost)
                {
                    finalHomeIdx = defaultHomeIdx;
                    finalAwayIdx = defaultAwayIdx;
                }
                else if (!mustHomeHost && mustAwayHost)
                {
                    finalHomeIdx = defaultAwayIdx;
                    finalAwayIdx = defaultHomeIdx;
                }
                else if (mustHomeHost && mustAwayHost)
                {
                    continue;
                }
                else
                {
                    if (homeCountH > homeCountA)
                    {
                        finalHomeIdx = defaultAwayIdx;
                        finalAwayIdx = defaultHomeIdx;
                    }
                    else
                    {
                        finalHomeIdx = defaultHomeIdx;
                        finalAwayIdx = defaultAwayIdx;
                    }
                }
            }
            if (finalHomeIdx == -1)
            {
                printf("P1 Balance Error\n");
                continue;
            }

            int foundSlot = 0;
            int searchAttempts = 0;
            int temp_day = current_day;
            int temp_month = current_month;
            int temp_year = current_year;
            int temp_matchCount = matchCountToday;
            while (!foundSlot && searchAttempts < 365)
            {
                searchAttempts++;
                int dayIsFull = isDayFull(temp_day, temp_month, temp_year, temp_matchCount);
                if (!isDateUnavailable(teams[finalHomeIdx].homeground, temp_day, temp_month) &&
                    !dayIsFull &&
                    !hasRecentlyPlayedAway(teams[finalAwayIdx].name, current_day, current_month, current_year))
                {
                    foundSlot = 1;
                    current_day = temp_day;
                    current_month = temp_month;
                    current_year = temp_year;
                    matchCountToday = temp_matchCount;
                }
                else
                {
                    incrementDate(&temp_day, &temp_month, temp_year);
                    temp_matchCount = 0;
                }
            }

            if (!foundSlot)
            {
                continue;
            }

            scheduleMatch(finalHomeIdx, finalAwayIdx, current_day, current_month, current_year, matchCountToday);
            scheduledMatchesCount++;
            matchCountToday++;
            if (isDayFull(current_day, current_month, current_year, matchCountToday))
            {
                incrementDate(&current_day, &current_month, current_year);
                matchCountToday = 0;
            }
        }
    }

    //  Phase 2: Extra Matches
    int extraPairings[5][5][2] = {
        {{0, 5}, {1, 6}, {2, 7}, {3, 8}, {4, 9}}, {{0, 6}, {1, 7}, {2, 8}, {3, 9}, {4, 5}}, {{0, 7}, {1, 8}, {2, 9}, {3, 5}, {4, 6}}, {{0, 8}, {1, 9}, {2, 5}, {3, 6}, {4, 7}}, {{0, 9}, {1, 5}, {2, 6}, {3, 7}, {4, 8}}};
    int extraRounds = 5;
    int totalExtraPairs = extraRounds * (MAX_TEAMS / 2);
    int e = 0;
    int attempts_phase2 = 0;
    int max_attempts_phase2 = totalExtraPairs * MAX_TEAMS * 5;
    int extraPairingScheduled[5][5] = {0};
    int phase2ScheduledCount = 0;

    while (scheduledMatchesCount < totalMatchesTarget && attempts_phase2 < max_attempts_phase2)
    {
        attempts_phase2++;
        int start_e = e;
        int found_unscheduled_e = 0;
        do
        {
            int current_round = e / (MAX_TEAMS / 2);
            int current_idx = e % (MAX_TEAMS / 2);
            if (!extraPairingScheduled[current_round][current_idx])
            {
                found_unscheduled_e = 1;
                break;
            }
            e = (e + 1) % totalExtraPairs;
        } while (e != start_e);
        if (!found_unscheduled_e || phase2ScheduledCount >= totalExtraPairs)
        {
            if (phase2ScheduledCount < totalExtraPairs && scheduledMatchesCount < totalMatchesTarget)
            {
                printf("Warning P2: Could not find next unscheduled extra pair. phase2Scheduled=%d\n", phase2ScheduledCount);
            }
            break;
        }

        int round = e / (MAX_TEAMS / 2);
        int idx = e % (MAX_TEAMS / 2);
        int teamA_idx = extraPairings[round][idx][0];
        int teamB_idx = extraPairings[round][idx][1];
        char teamA_name[50];
        strcpy(teamA_name, teams[teamA_idx].name);
        char teamB_name[50];
        strcpy(teamB_name, teams[teamB_idx].name);

        // Determine Desired Reverse Fixture
        int desiredHomeIdx = -1, desiredAwayIdx = -1;
        int firstFixtureFound = 0;
        for (int k = scheduledMatchesCount - 1; k >= 0; k--)
        {
            if ((strcmp(matchDetails[k].team1, teamA_name) == 0 && strcmp(matchDetails[k].team2, teamB_name) == 0))
            {
                desiredHomeIdx = teamB_idx;
                desiredAwayIdx = teamA_idx;
                firstFixtureFound = 1;
                break;
            }
            else if ((strcmp(matchDetails[k].team1, teamB_name) == 0 && strcmp(matchDetails[k].team2, teamA_name) == 0))
            {
                desiredHomeIdx = teamA_idx;
                desiredAwayIdx = teamB_idx;
                firstFixtureFound = 1;
                break;
            }
        }
        if (!firstFixtureFound)
        {
            desiredHomeIdx = (teams[teamA_idx].homematch_count <= teams[teamB_idx].homematch_count) ? teamA_idx : teamB_idx;
            desiredAwayIdx = (desiredHomeIdx == teamA_idx) ? teamB_idx : teamA_idx;
        }

        // Apply Balancing
        int finalHomeIdx = -1, finalAwayIdx = -1;
        bool allowRelaxedBalancing = (scheduledMatchesCount >= totalMatchesTarget - 2);
        int homeCountH = teams[desiredHomeIdx].homematch_count;
        int awayCountH = teams[desiredHomeIdx].awaymatch_count;
        int homeCountA = teams[desiredAwayIdx].homematch_count;
        int awayCountA = teams[desiredAwayIdx].awaymatch_count;
        int canHomeHost = (homeCountH < targetHomeGames);
        int canAwayHost = (homeCountA < targetHomeGames);
        int mustHomeHost = (awayCountH >= targetAwayGames);
        int mustAwayHost = (awayCountA >= targetAwayGames);

        if (canHomeHost && !canAwayHost)
        {
            finalHomeIdx = desiredHomeIdx;
            finalAwayIdx = desiredAwayIdx;
        }
        else if (!canHomeHost && canAwayHost)
        {
            finalHomeIdx = desiredAwayIdx;
            finalAwayIdx = desiredHomeIdx;
        }
        else if (!canHomeHost && !canAwayHost)
        {
            if (!allowRelaxedBalancing)
            {
                e = (e + 1) % totalExtraPairs;
                continue;
            }
            else
            {

                finalHomeIdx = desiredHomeIdx;
                finalAwayIdx = desiredAwayIdx;
            }
        }
        else
        {
            if (mustHomeHost && !mustAwayHost)
            {
                finalHomeIdx = desiredHomeIdx;
                finalAwayIdx = desiredAwayIdx;
            }
            else if (!mustHomeHost && mustAwayHost)
            {
                finalHomeIdx = desiredAwayIdx;
                finalAwayIdx = desiredHomeIdx;
            }
            else if (mustHomeHost && mustAwayHost)
            {
                if (!allowRelaxedBalancing)
                {
                    e = (e + 1) % totalExtraPairs;
                    continue;
                }
                else
                {

                    finalHomeIdx = desiredHomeIdx;
                    finalAwayIdx = desiredAwayIdx;
                }
            }
            else
            {
                finalHomeIdx = desiredHomeIdx;
                finalAwayIdx = desiredAwayIdx;
            }
        }
        if (finalHomeIdx == -1)
        {
            printf("P2 Balance Error e=%d\n", e);
            e = (e + 1) % totalExtraPairs;
            continue;
        }

        // Find Date Slot
        // Find Date Slot
        int foundSlot = 0;
        int searchAttempts = 0;
        int temp_day = current_day;
        int temp_month = current_month;
        int temp_year = current_year;
        int temp_matchCount = matchCountToday;
        while (!foundSlot && searchAttempts < 365 * 2)
        {
            searchAttempts++;
            if (temp_year > start_year || temp_month > end_month || (temp_month == end_month && temp_day > end_day))
            {
                break;
            }

            int dayIsFull = isDayFull(temp_day, temp_month, temp_year, temp_matchCount);
            int homeGapOk = !hasRecentlyPlayedAway(teams[finalHomeIdx].name, temp_day, temp_month, temp_year);
            int awayGapOk = !hasRecentlyPlayedAway(teams[finalAwayIdx].name, temp_day, temp_month, temp_year);
            if (homeGapOk && awayGapOk && !isDateUnavailable(teams[finalHomeIdx].homeground, temp_day, temp_month) && !dayIsFull)
            {
                foundSlot = 1;
                current_day = temp_day;
                current_month = temp_month;
                current_year = temp_year;
                matchCountToday = temp_matchCount;
            }
            else
            {
                incrementDate(&temp_day, &temp_month, temp_year);
                temp_matchCount = 0;
            }
        }

        if (!foundSlot)
        {
            e = (e + 1) % totalExtraPairs;
            continue;
        }

        // Schedule & Update
        scheduleMatch(finalHomeIdx, finalAwayIdx, current_day, current_month, current_year, matchCountToday);
        scheduledMatchesCount++;
        matchCountToday++;
        extraPairingScheduled[round][idx] = 1;
        phase2ScheduledCount++;

        if (isDayFull(current_day, current_month, current_year, matchCountToday))
        {
            incrementDate(&current_day, &current_month, current_year);
            matchCountToday = 0;
        }

        e = (e + 1) % totalExtraPairs;

       
    }
    printf("\nTotal matches scheduled: %d\n", scheduledMatchesCount);
}

// Function to display the match schedule
void displaySchedule()
{
    printf("\nIPL Match Schedule:\n");
    printf("-------------------------------------------------------------------------------\n");
    printf("%-3s %-6s %-3s %-6s %-40s %-10s %-15s\n", "No", "Team1", "vs", "Team2", "Venue", "Date", "Time");
    printf("-------------------------------------------------------------------------------\n");
    for (int i = 0; i < matchIndex; i++)
    {
        printf("%-3d %-6s %-3s %-6s %-40s %02d/%02d %-15s\n",
               i + 1, matchDetails[i].team1, "vs", matchDetails[i].team2,
               matchDetails[i].venue, matchDetails[i].day,
               matchDetails[i].month, matchDetails[i].time);
    }
    printf("-------------------------------------------------------------------------------\n");
}

// function to write schedule to CSV
void writeScheduleToCSV(const char *filename)
{
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        perror("Could not open file");
        return;
    }
    fprintf(file, "team1,team2,venue,day,month,time\n");
    for (int i = 0; i < matchIndex; i++)
    {
        fprintf(file, "%s,%s,%s,%d,%d,%s\n",
                matchDetails[i].team1, matchDetails[i].team2,
                matchDetails[i].venue, matchDetails[i].day,
                matchDetails[i].month, matchDetails[i].time);
    }
    fclose(file);
    printf("Schedule written to %s\n", filename);
}

// Function to find or add a team in the points table
int findOrAddTeam(char *teamName)
{
    for (int i = 0; i < teamCount; i++)
    {
        if (strcmp(pointsTable[i].team, teamName) == 0)
        {
            return i;
        }
    }

    if (teamCount < MAX_TEAMS)
    {
        strcpy(pointsTable[teamCount].team, teamName);
        pointsTable[teamCount].matchesPlayed = 0;
        pointsTable[teamCount].wins = 0;
        pointsTable[teamCount].losses = 0;
        pointsTable[teamCount].ties = 0;
        pointsTable[teamCount].points = 0;
        pointsTable[teamCount].totalRunsScored = 0;
        pointsTable[teamCount].totalOversFaced = 0.0;
        pointsTable[teamCount].totalRunsConceded = 0;
        pointsTable[teamCount].totalOversBowled = 0.0;
        pointsTable[teamCount].netRunRate = 0.0;
        return teamCount++;
    }
    else
    {
        printf("Error: Max teams reached, cannot add %s\n", teamName);
        return -1;
    }
}

// Function to calculate total runs/overs and then the final NRR for all teams
void calculateTotalsAndNRR()
{
    for (int i = 0; i < teamCount; i++)
    {
        pointsTable[i].totalRunsScored = 0;
        pointsTable[i].totalOversFaced = 0.0;
        pointsTable[i].totalRunsConceded = 0;
        pointsTable[i].totalOversBowled = 0.0;
        pointsTable[i].netRunRate = 0.0;
        pointsTable[i].matchesPlayed = pointsTable[i].wins + pointsTable[i].losses + pointsTable[i].ties;
    }
    for (int i = 0; i < matchIndex; i++)
    {
        int index1 = findOrAddTeam(matchDetails[i].team1);
        int index2 = findOrAddTeam(matchDetails[i].team2);
        if (index1 == -1 || index2 == -1)
        {
            printf("Error finding team indices for match %d. Skipping aggregation.\n", i + 1);
            continue;
        }
        pointsTable[index1].totalRunsScored += matchDetails[i].team1_runs;
        pointsTable[index1].totalOversFaced += matchDetails[i].team1_overs;
        pointsTable[index1].totalRunsConceded += matchDetails[i].team2_runs;
        pointsTable[index1].totalOversBowled += matchDetails[i].team2_overs;

        pointsTable[index2].totalRunsScored += matchDetails[i].team2_runs;
        pointsTable[index2].totalOversFaced += matchDetails[i].team2_overs;
        pointsTable[index2].totalRunsConceded += matchDetails[i].team1_runs;
        pointsTable[index2].totalOversBowled += matchDetails[i].team1_overs;
    }

    for (int i = 0; i < teamCount; i++)
    {
        float runRateScored = 0.0;
        float runRateConceded = 0.0;

        if (pointsTable[i].totalOversFaced > 0.001)
        {

            runRateScored = (float)pointsTable[i].totalRunsScored / pointsTable[i].totalOversFaced;
        }

        if (pointsTable[i].totalOversBowled > 0.001)
        {
            runRateConceded = (float)pointsTable[i].totalRunsConceded / pointsTable[i].totalOversBowled;
        }
        pointsTable[i].netRunRate = runRateScored - runRateConceded;
    }
    printf("NRR calculation complete.\n");
}

// Function to update points table based on match results, including runs and overs
void getMatchResults()
{
    int team1Runs, team2Runs, winnerInput;
    float team1Overs, team2Overs;
    printf("\n\nEnter match results:-\n");
    teamCount = 0;
    for (int i = 0; i < matchIndex; i++)
    {
        printf("Match %d: %s(1) vs %s(2) at %s on %02d/%02d\n",
               i + 1,
               matchDetails[i].team1,
               matchDetails[i].team2,
               matchDetails[i].venue,
               matchDetails[i].day,
               matchDetails[i].month);
        int index1 = findOrAddTeam(matchDetails[i].team1);
        int index2 = findOrAddTeam(matchDetails[i].team2);
        if (index1 == -1 || index2 == -1)
        {
            printf("Error finding team index for match %d. Skipping result entry.\n", i + 1);
            continue;
        }
        // Team 1 Input
        printf("Enter runs scored by %s: ", matchDetails[i].team1);
        scanf("%d", &team1Runs);
        while (team1Runs < 0 ||
               (team1Runs < 50 && ({ 
          char confirm; 
          printf("You entered less than 50 runs for %s. Are you sure? (y/n): ", matchDetails[i].team1); 
          scanf(" %c", &confirm); 
          confirm != 'y' && confirm != 'Y'; })) || (team1Runs > 400 && ({ 
          char confirm; 
          printf("You entered more than 400 runs for %s. Are you sure? (y/n): ", matchDetails[i].team1); 
          scanf(" %c", &confirm); 
          confirm != 'y' && confirm != 'Y'; })))
        {
            printf("Invalid or suspicious input. Please re-enter runs for %s: ", matchDetails[i].team1);
            scanf("%d", &team1Runs);
        }

        printf("Enter overs played by %s: ", matchDetails[i].team1);
        scanf("%f", &team1Overs);
        while (team1Overs < 0 || team1Overs > 20)
        {
            printf("Overs should be between 0 and 20. Enter again for %s: ", matchDetails[i].team1);
            scanf("%f", &team1Overs);
        }

        // Team 2 Input
        printf("Enter runs scored by %s: ", matchDetails[i].team2);
        scanf("%d", &team2Runs);
        while (team2Runs < 0 ||
               (team2Runs < 50 && ({ 
          char confirm; 
          printf(" You entered less than 50 runs for %s. Are you sure? (y/n): ", matchDetails[i].team2); 
          scanf(" %c", &confirm); 
          confirm != 'y' && confirm != 'Y'; })) || (team2Runs > 400 && ({ 
          char confirm; 
          printf("You entered more than 400 runs for %s. Are you sure? (y/n): ", matchDetails[i].team2); 
          scanf(" %c", &confirm); 
          confirm != 'y' && confirm != 'Y'; })))
        {
            printf("Invalid or suspicious input. Please re-enter runs for %s: ", matchDetails[i].team2);
            scanf("%d", &team2Runs);
        }

        printf("Enter overs played by %s: ", matchDetails[i].team2);
        scanf("%f", &team2Overs);
        while (team2Overs < 0 || team2Overs > 20)
        {
            printf("Overs should be between 0 and 20. Enter again for %s: ", matchDetails[i].team2);
            scanf("%f", &team2Overs);
        }

        matchDetails[i].team1_runs = team1Runs;
        matchDetails[i].team1_overs = team1Overs;
        matchDetails[i].team2_runs = team2Runs;
        matchDetails[i].team2_overs = team2Overs;

        if (team1Runs > team2Runs)
        {
            matchDetails[i].winner = 1;
            printf("%s Won\n", matchDetails[i].team1);
        }
        else if (team2Runs > team1Runs)
        {
            matchDetails[i].winner = 2;
            printf("%s Won\n", matchDetails[i].team2);
        }
        else
        {
            matchDetails[i].winner = 0;
            printf("Match Tied\n");
        }

        pointsTable[index1].matchesPlayed++;
        pointsTable[index2].matchesPlayed++;

        if (matchDetails[i].winner == 1)
        {
            pointsTable[index1].wins++;
            pointsTable[index1].points += 2;
            pointsTable[index2].losses++;
        }
        else if (matchDetails[i].winner == 2)
        {
            pointsTable[index2].wins++;
            pointsTable[index2].points += 2;
            pointsTable[index1].losses++;
        }
        else
        {
            pointsTable[index1].ties++;
            pointsTable[index2].ties++;
            pointsTable[index1].points += 1;
            pointsTable[index2].points += 1;
        }
        system("cls");
    }
}

// Function to sort the points table
void sortPointsTable()
{
    for (int i = 0; i < teamCount - 1; i++)
    {
        int max_index = i;
        for (int j = i + 1; j < teamCount; j++)
        {
            if (pointsTable[j].points > pointsTable[max_index].points ||
                (pointsTable[j].points == pointsTable[max_index].points &&
                 pointsTable[j].netRunRate > pointsTable[max_index].netRunRate))
            {
                max_index = j;
            }
        }
        if (max_index != i)
        {
            PointsTable temp = pointsTable[i];
            pointsTable[i] = pointsTable[max_index];
            pointsTable[max_index] = temp;
        }
    }
}

// Function to display the points table
void displayPointsTable()
{
    printf("\nPoints Table:\n");
    printf("--------------------------------------------------------------------\n");
    printf("%-20s %-3s %-4s %-4s %-4s %-7s %-10s\n", "Team", "MP", "W", "L", "T", "Points", "NRR");
    printf("--------------------------------------------------------------------\n");

    for (int i = 0; i < teamCount; i++)
    {
        printf("%-20s %-3d %-4d %-4d %-4d %-7d %-10.3f\n",
               pointsTable[i].team,
               pointsTable[i].matchesPlayed,
               pointsTable[i].wins,
               pointsTable[i].losses,
               pointsTable[i].ties,
               pointsTable[i].points,
               pointsTable[i].netRunRate);
    }
    printf("--------------------------------------------------------------------\n");
}

// Function to display the playoffs
void displayPlayoffs()
{
    printf("\nTop 4 Teams Qualifying for Playoffs:\n");
    for (int i = 0; i < 4; i++)
    {
        printf("%d. %s\n", i + 1, pointsTable[i].team);
    }
    char top4[4][50];
    strcpy(top4[0], pointsTable[0].team);
    strcpy(top4[1], pointsTable[1].team);
    strcpy(top4[2], pointsTable[2].team);
    strcpy(top4[3], pointsTable[3].team);

    char inputWinner[50];

    char q1_winner[50], q1_loser[50];
    printf("\nQualifier 1: %s vs %s\n", top4[0], top4[1]);
    printf("Enter winner of Qualifier 1: ");
    scanf("%s", inputWinner);
    strcpy(q1_winner, inputWinner);
    if (strcmp(q1_winner, top4[0]) == 0)
        strcpy(q1_loser, top4[1]);
    else
        strcpy(q1_loser, top4[0]);
    printf("Winner: %s (directly qualifies for the Final)\n", q1_winner);
    printf("Loser: %s (gets a second chance in Qualifier 2)\n\n", q1_loser);

    char elim_winner[50], elim_loser[50];
    printf("Eliminator: %s vs %s\n", top4[2], top4[3]);
    printf("Enter winner of the Eliminator: ");
    scanf("%s", inputWinner);
    strcpy(elim_winner, inputWinner);
    if (strcmp(elim_winner, top4[2]) == 0)
        strcpy(elim_loser, top4[3]);
    else
        strcpy(elim_loser, top4[2]);
    printf("Winner: %s (advances to Qualifier 2)\n", elim_winner);
    printf("Loser: %s (eliminated from the tournament)\n\n", elim_loser);

    char q2_winner[50], q2_loser[50];
    printf("Qualifier 2: %s vs %s\n", q1_loser, elim_winner);
    printf("Enter winner of Qualifier 2: ");
    scanf("%s", inputWinner);
    strcpy(q2_winner, inputWinner);
    if (strcmp(q2_winner, q1_loser) == 0)
        strcpy(q2_loser, elim_winner);
    else
        strcpy(q2_loser, q1_loser);
    printf("Winner: %s (qualifies for the Final)\n", q2_winner);
    printf("Loser: %s (eliminated from the tournament)\n\n", q2_loser);

    char final_winner[50];
    printf("Final: %s vs %s\n", q1_winner, q2_winner);
    printf("Enter champion of the Final: ");
    scanf("%s", inputWinner);
    strcpy(final_winner, inputWinner);
    printf("Champion: %s\n", final_winner);
}

void writeMatchDetailsToCSV(const char *filename)
{
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        perror("Error opening match details CSV file for writing");
        return;
    }
    fprintf(file, "MatchNo,Date,Time,HomeTeam,AwayTeam,Venue,HomeRuns,HomeOvers,AwayRuns,AwayOvers,Result\n");
    for (int i = 0; i < matchIndex; i++)
    {
        fprintf(file, "%d,%02d/%02d,%s,%s,%s,%s",
                i + 1,
                matchDetails[i].day,
                matchDetails[i].month,
                matchDetails[i].time,
                matchDetails[i].team1,
                matchDetails[i].team2,
                matchDetails[i].venue);

        char resultStr[60] = "N/A";

        if (matchDetails[i].team1_runs > 0 || matchDetails[i].team2_runs > 0 ||
            matchDetails[i].team1_overs > 0 || matchDetails[i].team2_overs > 0)
        {
            if (matchDetails[i].team1_runs > matchDetails[i].team2_runs)
            {
                snprintf(resultStr, sizeof(resultStr), "%s Won", matchDetails[i].team1);
            }
            else if (matchDetails[i].team2_runs > matchDetails[i].team1_runs)
            {
                snprintf(resultStr, sizeof(resultStr), "%s Won", matchDetails[i].team2);
            }
            else
            {
                snprintf(resultStr, sizeof(resultStr), "Tie");
            }
        }

        fprintf(file, ",%d,%.1f,%d,%.1f,%s\n",
                matchDetails[i].team1_runs,
                matchDetails[i].team1_overs,
                matchDetails[i].team2_runs,
                matchDetails[i].team2_overs,
                resultStr);
    }

    fclose(file);
    printf("Match details successfully written to %s\n", filename);
}

void writePointsTableToCSV(const char *filename)
{

    FILE *file = fopen(filename, "w");
    if (!file)
    {
        perror("Error opening points table CSV file for writing");
        return;
    }
    fprintf(file, "Position,Team,MatchesPlayed,Wins,Losses,Points,NRR\n");
    for (int i = 0; i < teamCount; i++)
    {

        fprintf(file, "%d,%s,%d,%d,%d,%d,%.3f\n",
                i + 1,
                pointsTable[i].team,
                pointsTable[i].matchesPlayed,
                pointsTable[i].wins,
                pointsTable[i].losses,
                pointsTable[i].points,
                pointsTable[i].netRunRate);
    }

    fclose(file);
    printf("Points Table successfully written to %s\n", filename);
}

int main()
{
    inputDates();
    //inputTeamsAndGrounds();
    inputVenueAvailability();
    matchScheduling();
    displaySchedule();
    writeScheduleToCSV("ipl_schedule.csv");
    getMatchResults();
    writeMatchDetailsToCSV("matches.csv");
    calculateTotalsAndNRR();
    sortPointsTable();
    displayPointsTable();
    writePointsTableToCSV("points_table.csv");
    displayPlayoffs();
    return 0;
}