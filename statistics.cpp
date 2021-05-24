#include "statistics.hpp"

#include <iomanip>
#include <sstream>
#include <string>
#include <unordered_map>

static std::unordered_map<std::string, UserStats> stats;

static std::string getDate()
{
    std::time_t currentTime = std::time(nullptr);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&currentTime), "%d-%m-%Y");
    return ss.str();
}

void addBrushingData(const std::string &name, BrushingData data)
{
    if (stats.count(name) == 0)
    {
        stats[name] = UserStats{};
    }

    UserStats &userStats = stats[name];

    auto today = getDate();

    if (userStats.lastBrushingDate == today)
    {
        userStats.oneYearHistory.back().push_back(data);
    }
    else
    {
        std::vector<BrushingData> newVector = {data};
        userStats.oneYearHistory.push_back(newVector);
    }

    userStats.lastBrushingDate = today;
}

json getStatistics(const std::string &name)
{
    const auto &userStats = stats.at(name);
    const auto &oneYearHistory = userStats.oneYearHistory;

    int timeMonth = 0;
    int timeYear = 0;
    int bleedingMonth = 0;
    int bleedingYear = 0;
    //nr periaje pe zi difera, folosesc countere pt a calcula mediile
    int nrMonth = 0;
    int nrYear = 0;

    for (unsigned int i = 0; i < oneYearHistory.size(); i++)
    {
        for (unsigned int j = 0; j < oneYearHistory[i].size(); j++)
        {
            nrYear++;
            BrushingData p = oneYearHistory[i][j];
            timeYear += p.time;
            if (p.bleeding)
            {
                bleedingYear++;
                if (oneYearHistory.size() - i <= 30)
                {
                    bleedingMonth++;
                }
            }
            if (oneYearHistory.size() - i <= 30)
            {
                nrMonth++;
            }
        }
    }

    timeMonth /= nrMonth;
    timeYear /= nrYear;
    bleedingMonth /= nrMonth;
    bleedingMonth *= 100; //procentual
    bleedingYear /= nrYear;
    bleedingYear *= 100;

    int minimBrushings = 9999;
    int maximBrushings = -1;

    auto t = oneYearHistory.back().back().teethWithTartrum;
    auto tartrumHistory = std::vector<int>(50);

    for (unsigned int i = 0; i < t.size(); i++)
    {
        tartrumHistory[t[i]]++; //daca am gasit tartru, cresc nr de periaje necesare
    }
    for (int i = 0; i < 32; i++)
    {
        if (find(t.begin(), t.end(), i) == t.end() && tartrumHistory[i] > 0) //daca dintele e curat, dar la periajele trecute nu era
        {
            //statistica legata de tartru este de forma
            //"Sunt necesare intre minimPeriaje si maximPeriaje pentru a elimina tartrul de pe un dinte."

            if (tartrumHistory[i] < minimBrushings)
                minimBrushings = tartrumHistory[i];
            if (tartrumHistory[i] > maximBrushings)
                maximBrushings = tartrumHistory[i];
            tartrumHistory[i] = 0; //resetam counterul
        }
    }

    json j;

    j["lastMonth"]["brushingTime"] = timeMonth;
    j["lastMonth"]["bleedingPercent"] = bleedingMonth;

    j["lastYear"]["brushingTime"] = timeYear;
    j["lastYear"]["bleedingPercent"] = bleedingYear;

    j["tartrumRemoval"]["min"] = minimBrushings;
    j["tartrumRemoval"]["max"] = maximBrushings;

    return j;
}
