#include "directions.hpp"

#include "brushing.hpp"
#include "configuration.hpp"

std::pair<LedsColor, Direction> getDirection()
{
    const Configuration &config = *getCurrentConfiguration();

    return std::make_pair(LedsColor::Red, Direction::Turn_Off);
}

std::vector<vector<unsigned>> getDirections(Brushing &brush, int index)
{
    std::vector<int>::iterator it = std::find(bleeds.begin(), bleeds.end(), index);
    if (it == bleeds.end())
    {
        //dintele sangereaza
        brush.Leds.push_back("Red");
        brush.Graphic_Directions.push_back("X");
    }
    if ((index == 1 || index == 32) || (index > 1 && index <= 8) || (index >= 25 && index < 32)) //posib ma aflu la primul dinte de sus sau la ultimul de jos, trebuie sa merg spre stanga
    {
        vector<vector<unsigned>> matrix{
            {0, 0, 0, 0, 1, 0, 0, 0},
            {0, 0, 0, 0, 0, 1, 0, 0},
            {0, 0, 0, 0, 0, 0, 1, 0},
            {1, 1, 1, 1, 1, 1, 1, 1},
            {0, 0, 0, 0, 0, 0, 1, 0},
            {0, 0, 0, 0, 0, 1, 0, 0},
            {0, 0, 0, 0, 1, 0, 0, 0}};
        brush.Leds.push_back("Purple");
        brush.Graphic_Directions.push_back("==>");
        brush.Directions.push_back("Right");
        return matrix;
    }
    if ((index == 16 || index == 17) || (index > 17 && index <= 24) || (index >= 9 && index <= 16)) //posib sa ma aflu la ultimul dinte de sus sau la primul de jos, deci dreapta
    {
        vector<vector<unsigned>> matrix{
            {0, 0, 0, 1, 0, 0, 0, 0},
            {0, 0, 1, 0, 0, 0, 0, 0},
            {0, 1, 0, 0, 0, 0, 0, 0},
            {1, 1, 1, 1, 1, 1, 1, 1},
            {0, 1, 0, 0, 0, 0, 0, 0},
            {0, 0, 1, 0, 0, 0, 0, 0},
            {0, 0, 0, 1, 0, 0, 0, 0}};
        brush.Leds.push_back("Blue");
        brush.Graphic_Directions.push_back("<==");
        brush.Directions.push_back("Left");
        return matrix;
    }
// daca nu e una din variantele mentionate in ifuri, am ales in mod arbitrara directia
}

void setDirections(Config &configuration)
{
    selectUserStats(configuration.stats);

    Brushing *currentBrushing = new Brushing();
    std::cout << "User name:" << configuration.Name << '\n';
    std::string returnString = "";
    std::vector<int> verif(configuration.STeeth.size(), 0);
    startCronometru(); //porneste cronometrul pentru periere

    if ((int)configuration.program == 1 || (int)configuration.program == 2)
    {
        //periere completa sau de sus, incep dinte 9->16 apoi 1->8, 24->17, 25->32
        for (unsigned i = 9; i <= 16; i++)
        {
            returnString += "Tooth ";
            returnString += i + "\n";
            vector<vector<unsigned>> mtx = getDirections(*currentBrushing, i);
            for (unsigned j = 0; j < 7; j++)
            {
                for (unsigned k = 0; k < 8; k++)
                {
                    if (mtx[j][k])
                        returnString += "* ";
                    else
                        returnString += " ";
                }
                returnString += "\n";
            }
        }
        for (unsigned i = 8; i <= 1; i--)
        {
            //8 -> 1
            returnString += "Tooth ";
            returnString += i + "\n";
            vector<vector<unsigned>> mtx = getDirections(*currentBrushing, i);
            for (unsigned j = 0; j < 7; j++)
            {
                for (unsigned k = 0; k < 8; k++)
                {
                    if (mtx[j][k])
                        returnString += "* ";
                    else
                        returnString += " ";
                }
                returnString += "\n";
            }
        }
    }
    if ((int)configuration.program == 1 || (int)configuration.program == 3)
    {
        //periati dintii de jos acum 24->17
        for (unsigned i = 24; i >= 17; i--)
        {
            returnString += "Tooth ";
            returnString += i + "\n";
            vector<vector<unsigned>> mtx = getDirections(*currentBrushing, i);
            for (unsigned j = 0; j < 7; j++)
            {
                for (unsigned k = 0; k < 8; k++)
                {
                    if (mtx[j][k])
                        returnString += "* ";
                    else
                        returnString += " ";
                }
                returnString += "\n";
            }
        }
        for (unsigned i = 25; i <= 32; i--)
        {
            //25->32
            returnString += "Tooth ";
            returnString += i + "\n";
            vector<vector<unsigned>> mtx = getDirections(*currentBrushing, i);
            for (unsigned j = 0; j < 7; j++)
            {
                for (unsigned k = 0; k < 8; k++)
                {
                    if (mtx[j][k])
                        returnString += "* ";
                    else
                        returnString += " ";
                }
                returnString += "\n";
            }
        }
    }
    if ((int)configuration.program == 4)
    {
        currentBrushing->Leds.push_back("Grey");
    }

    std::string Statistics = statistics();
    saveUserStats(configuration.stats);
    returnString += Statistics;
    currentBrushing->History.push_back(returnString);
    saved_Brushing.push_back(currentBrushing);
}

void getDirections(const Rest::Request &request, Http::ResponseWriter response)
{
    std::string returnString = "No brushing available!";
    unsigned j = 0;
    for (Brushing *&currBrushing : saved_Brushing)
    {
        returnString = "";
        for (unsigned i = 0; i < currBrushing->Directions.size(); ++i)
        {
            returnString += "Colour is ";
            returnString += currBrushing->Leds[i];
            returnString += ", direction is ";
            returnString += currBrushing->Directions[i];
            //returnString += currBrushing->Graphic_Directions[i]; //pt ca bratara
            returnString += '\n';
            returnString += "================================";
        }
        returnString += currBrushing->History[j];
        returnString += '\n';
        returnString += "======================================= \n";
    }

    response.send(Http::Code::Ok, returnString.c_str());
}
