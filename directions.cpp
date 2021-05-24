#include "directions.hpp"

#include "brushing.hpp"
#include "configuration.hpp"

std::pair<LedsColor, Direction> getDirection()
{
    const Configuration &config = *getCurrentConfiguration();

    return std::make_pair(LedsColor::Red, Direction::Turn_Off);
}

/*
int getLedsColorFromString(string &s)
{
	// Grey = 1,
	// Blue,
	// Green,
	// Red
    string str;
    str = s;
    for (int x = 0; x < str.length(); x++)
        s[x] = toupper(str[x]);
    if (str.compare("GREY") == 0)
        return 1;
    if (str.compare("BLUE") == 0)
        return 2;
    if (str.compare("GREEN") == 0)
        return 3;
    if (str.compare("RED") == 0)
        return 4;

    return -1;
}

std::string getDirectionFromNumber(int nr)
{
	// No_Direction = 1,
	// Right,
	// Left,
	// Turn_Off,
	// Warning_Wrong_Direction
    switch (nr)
    {
    case 1:
        return "No Direction";
    case 2:
        return "To Right";
    case 3:
        return "To Left";
    case 4:
        return "Turn Off";
    default:
        return "Unknown input";
    }
}

std::vector<string> Colors;

////configuratia va fi luata de la utilizator, iar perierea se va face in functie de dintii salvati pentru utilizator
//si de culoarea cu care ii definim pe acestia
void setDirectionAndLedsGraphic(Config &configuration)
{
    //Config configuration = getConfigure;
    //afla cum faci rost de configuratie din request direct

    selectUserStats(configuration.stats);

    Brushing *currentBrushing = new Brushing();
    std::cout << "User name:" << configuration.Name << '\n';
    int nb_teeth;
    int need_brushing = 0;
    pair<int, int> current_teeth;
    int det_dirct;
    std::string returnString = "No colors for leds."; //retin ceea ce trebuie sa apra in istoric
    //pentru a afisa pe bratara, voi afisa efectiv semne pentru a marca directia, sageti, x sau o o sa ti dai seama pe parcurs
    std::string outputBrace = "O";

    //verificare ce zona trebuie periata conf programului ales
    if ((int)configuration.program == 1)
    {
        //daca trebuie periere completa, pp ca periajul incepe de la cei 2 incisivi de sus
        current_teeth.first = configuration.STeeth[configuration.STeeth.size() / 4]; //am folosit peste tot steeth.size pentru ca m am gandit ca poate unii au mai putini dinti, gen copiii carora le au cazut, daca ne complicam prea mult cu asta, modific cu masuri exacte
        current_teeth.second = configuration.STeeth[configuration.STeeth.size() / 4] + 1;
        nb_teeth = configuration.STeeth.size();
        det_dirct = 0;
    }
    else if ((int)configuration.program == 2)
    {
        // daca trebuie periere doar sus, pp ca periajul incepe de la cei 2 incisivi de sus
        current_teeth.first = configuration.STeeth[configuration.STeeth.size() / 4];
        current_teeth.second = configuration.STeeth[configuration.STeeth.size() / 4] + 1;
        nb_teeth = configuration.STeeth.size() / 2;
        det_dirct = 1;
    }
    else if ((int)configuration.program == 3)
    {
        //trebuie periati doar cei de jos, presupunem ca periajul incepe de la cei doi incisivi de jos
        current_teeth.first = configuration.STeeth[configuration.STeeth.size() - configuration.STeeth[configuration.STeeth.size() / 4]];
        current_teeth.second = configuration.STeeth[configuration.STeeth.size() - configuration.STeeth[configuration.STeeth.size() / 4] + 1];
        nb_teeth = configuration.STeeth.size() / 2;
        det_dirct = -1;
    }
    else
    {
        // dintii nu mai trebuie periati
        need_brushing = 1;
        current_teeth.first = 1;
        current_teeth.second = 2;
        nb_teeth = 0;
    }

    //au fost periati dintii corespunzatori inceputului
    std::vector<int> verif(configuration.STeeth.size(), need_brushing);
    verif[current_teeth.first] = 1;
    verif[current_teeth.second] = 1;

    if (Colors.size())
        returnString = "";

    for (int i; i < Colors.size(); i++)
    {
        startCronometru(); //porneste cronometrul pentru periere
        //salvez culorile ledurilor si directia pe care o indica
        currentBrushing->Leds.push_back(Colors[i]);
        currentBrushing->Directions.push_back(getDirectionFromNumber(getLedsColorFromString(Colors[i])));

        //calculez ce inseamna directiile in functie de perierea care are loc
        if (getLedsColorFromString(Colors[i]) == 1)
        {
            if (!(std::count(verif.begin(), verif.end(), 0)))
            {
                returnString += "\n Completely brushing!";
            }
            else
            {
                //nu avem senzori aprinsi, dar avem ce dinti peria
                outputBrace = "O"; //output pentru bratara aka ledul este gri nu se periaza, vezi cum faci output pt bratara asta
                currentBrushing->Graphic_Directions.push_back(outputBrace);
                vector<int>::iterator x = std::find(verif.begin(), verif.end(), 0);
                //x = std::find(verif.begin(), verif.end(), 0);
                while (x != verif.end())
                {
                    // avem ce dinti peria
                    returnString += "You brushing " + *x;
                    returnString += " and " + ((*x) + 1);
                    returnString += " teeth \n";
                    verif[*x] = 1;
                    verif[(*x) + 1] = 1;
                    x = std::find(verif.begin(), verif.end(), 0);
                }
            }
        }
        else
        {
            if (getLedsColorFromString(Colors[i]) == 4)
            {
                // trebuie dat off pentru ca utilizatorul sangereaza, deci se revine la pozitia initiala de periere
                outputBrace = "X"; //apare conditie de stop pe bratara pentru sangerare
                currentBrushing->Graphic_Directions.push_back(outputBrace);
                if (det_dirct == 0 || det_dirct == 1)
                {
                    returnString += "Your " + current_teeth.first;
                    returnString += " and " + current_teeth.second;
                    returnString += " teeth are bleeding.";
                    current_teeth.first = configuration.STeeth[configuration.STeeth.size() / 4];
                    current_teeth.second = configuration.STeeth[configuration.STeeth.size() / 4] + 1;
                    returnString += " After you rinse your mouth, we restart the programm from " + current_teeth.first;
                    returnString += " and " + current_teeth.second;
                    returnString += " teeth. \n";
                    // nu marcam dintele care periaza ca fiind periat deoarce vom reveni asupra lui in caz ca va fi nevoie
                }
            }
            else
            {
                int cst = getLedsColorFromString(Colors[i]);
                if (cst == 2) //dreapta, deci merg inapoi
                {
                    cst = -1;
                    outputBrace = "==>";
                    currentBrushing->Graphic_Directions.push_back(outputBrace);
                }
                if (cst == 3) //stanga, deci merg inainte
                {
                    cst = 1;
                    outputBrace = "<==";
                    currentBrushing->Graphic_Directions.push_back(outputBrace);
                }
                if (cst == -1 || cst == 1)
                {
                    //periere stg sau dreapta
                    if ((!det_dirct || det_dirct == 1))
                    {
                        //pornim de la incisivii de sus, adunam cst pentru ca daca e dreapta scade cu 1, iar daca e stg creste
                        // nu mai testez limitele pentru ca daca nu ar mai avea dinti, nu ar mai fi leduri colorate
                        current_teeth.first += cst;
                        current_teeth.second += cst;
                        verif[current_teeth.first] = 1;
                        verif[current_teeth.second] = 1;
                        returnString += "You brushing " + current_teeth.first;
                        returnString += " and " + current_teeth.second;
                        returnString += " teeth \n";
                    }
                    else if (det_dirct == -1)
                    {
                        //incepem de la incisivii de jos
                        current_teeth.first -= cst;
                        current_teeth.second -= cst;
                        verif[current_teeth.first] = 1;
                        verif[current_teeth.second] = 1;
                        returnString += "You brushing " + current_teeth.first;
                        returnString += " and " + current_teeth.second;
                        returnString += " teeth \n";
                    }
                }
                else
                {
                    returnString += "Colour " + Colors[i];
                    returnString += " doesn't exist! \n";
                }
            }
        }
    }

    //am terminat vectorul de leduri, nu mai sunt directii recomandate, dar totusi mai avem dinti neperiati
    vector<int>::iterator x = std::find(verif.begin(), verif.end(), 0);
    while (x != verif.end())
    {
        // avem ce dinti peria
        returnString += "You brushing " + *x;
        returnString += " and " + ((*x) + 1);
        returnString += " teeth \n";
        verif[*x] = 1;
        verif[(*x) + 1] = 1;
        x = std::find(verif.begin(), verif.end(), 0);
    }

    std::string Statistics = statistics();
    saveUserStats(configuration.stats);
    returnString += Statistics;
    currentBrushing->History.push_back(returnString); //se creaza istoricul cu dintii periati
    saved_Brushing.push_back(currentBrushing);
}
*/
