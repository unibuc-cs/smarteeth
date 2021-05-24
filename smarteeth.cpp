#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <utility>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <pistache/endpoint.h>
#include <pistache/router.h>
#include <nlohmann/json.hpp>
#include <mqtt/client.h>

using namespace std;
using namespace chrono;
using namespace Pistache;
using namespace Pistache::Http;
using namespace Pistache::Rest;

// Convenient namespace alias
using json = nlohmann::json;

class Toothbrush
{
public:
    Toothbrush()
    {
    }

};

Toothbrush brush;
enum eType
{
    Full_Clean = 1,
    Only_Upper,
    Only_Lower,
    Warning_Safe_Teeths_Full_Clean
};

struct brushingData //toate datele retinute in urma unui periaj
{
    int time;            //secunde
    vector<int> tartrum; //vector cu dintii afectati (am presupus ca notam fiecare dinte cu un int)
    bool bleeding;       //true daca s-a identificat o sangerare in timpul periajului
};

struct userStats
{
    vector<vector<brushingData>> oneYearHistory; //vector de zile; o zi are mai multe periaje
    int d, m, y;                                 //data ultimului periaj
    vector<int> tartrumHistory = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //default value, nu stiu daca e ok asa
    int minimBrushings = 0;
    int maximBrushings = 0;
};

struct Config
{
    std::string Name;
    int age;
    eType program;
    std::vector<int> STeeth; // dintii
    userStats stats;
};

std::vector<Config*> saved_Configs; //configuratii salvate
/*
Teeth Skema
maybe using enum in the near future and names to parse c1(canin 1) m1(molar 1) will see, will see
1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16
32 31 30 29 28 27 26 25 24 23 22 21 20 19 18  17
Source :
https://ibb.co/KhfB2Kp

da cuaie stiu ca am scris theets calmeaza-te nu urla

*/



enum LedsColor //culorile pe care le ia ledul pentru a stabili in ce directie urmeaza sa mearga utilizatorul
{
    Grey = 1, //nu este aprins, utilizatorul a terminat perierea indicata de leduri
    Blue,     //dreapta
    Purple,   //stanga
    Red      //stop - in cazul sangerarilor, se opreste pentru clatire
};

//pp ca sunt perioati cate 2 dinti concomitent
enum Directions
{
    No_Direction = 1,
    Right,
    Left,
    Turn_Off
};

struct Brushing
{
    std::vector<string> Leds;
    //ex: Leds = ["Blue", "Blue", "Blue", "Purple", "Purple", "Red", "Purple", "Grey"]
    std::vector<string> Directions; //directiile in cuvinte
    std::vector<string> Graphic_Directions; //asta se va afisa pe bratara
    std::vector<string> History; //ce dinti si cand au fost spalati
};

std::vector<Brushing*> saved_Brushing;

void helloRoute(const Rest::Request& request, Http::ResponseWriter response)
{
    // Read the request data
    cout << "Received a request from IP address " << request.address().host() << '\n';

    // Send a reply
    response.send(Http::Code::Ok, "Hello world!");
}

void setConfigure(const Rest::Request& request, Http::ResponseWriter response)
{
    Config* ourConfiguration;

    const auto params = json::parse(request.body());

    std::cout << "Input received: " << params << '\n';

    const auto name = params["name"];

    bool foundVar = false;
    for(Config* &oC : saved_Configs)
    {
        if(oC->Name == name)
        {
            foundVar = true;
            ourConfiguration = oC;
            ourConfiguration->STeeth.clear(); //ATENTIE!! Numele este unic, daca se primesc alte setari cu acelasi nume se considera update.
        }
    }

    if (!foundVar)
    {
        ourConfiguration = new Config();
        saved_Configs.push_back(ourConfiguration);
    }

    ourConfiguration->Name = name;
    ourConfiguration->age = params["age"];
    ourConfiguration->program = params["program"];

    for (int tooth : params["teeth"])
    {
        if (tooth < 1 || tooth > 32)
        {
            std::cout << "Ignoring invalid tooth " << tooth << '\n';
            continue;
        }

        // Elimina duplicatele
        if (find(ourConfiguration->STeeth.begin(), ourConfiguration->STeeth.end(), tooth)
            != ourConfiguration->STeeth.end())
        {
            std::cout << "Ignoring duplicate tooth " << tooth << '\n';
            continue;
        }

        ourConfiguration->STeeth.push_back(tooth);

    }

    response.send(Http::Code::Ok, "Configuration Saved!");
}

std::string getModelFromNumber(int p)
{
    /*
    Full_Clean = 1,
    Only_Upper,
    Only_Lower,
    Warning_Safe_Teeths_Full_Clean
    */
    switch (p)
    {
    case 1:
        return "Full_Clean";
    case 2:
        return "Only Upper";
    case 3:
        return "Only Lower";
    case 4:
        return "Warning Safe Teeth Full Clean";
    default:
        return "Unknown Config";
    }
}

void getConfigure(const Rest::Request& request, Http::ResponseWriter response)
{
    std::string returnString = "No Configuration Available!";
    auto TextParam = request.param(":configurationName").as<std::string>();
    for(Config* &oConfig : saved_Configs)
        if(oConfig->Name == TextParam)
        {
            returnString = "Name : " + TextParam + "\nAge : " + to_string(oConfig->age) + "\nModel setted on : " + getModelFromNumber((int)oConfig->program) + "\n";
            if (!oConfig->STeeth.empty())
            {
                returnString += "These teeth are supervised : ";
                for (unsigned int i = 0; i < oConfig->STeeth.size(); ++i)
                    returnString += to_string(oConfig->STeeth[i]) + " ";
            }
        }
    response.send(Http::Code::Ok, returnString.c_str());
}

//-----------------------------------------------------------

vector<vector<brushingData>> oneYearHistory;
int d, m, y;
int nd, nm, ny;
vector<int> tartrumHistory;
auto start = high_resolution_clock::now();
auto finish = high_resolution_clock::now();
int minimBrushings;
int maximBrushings;

void selectUserStats(userStats i) //salvez in variabilele globale datele userului curent
{
    oneYearHistory = i.oneYearHistory;
    d = i.d;
    m = i.m;
    y = i.y;
    tartrumHistory = i.tartrumHistory;
    minimBrushings = i.minimBrushings;
    maximBrushings = i.maximBrushings;
}

void saveUserStats(userStats &i) //lucrez cu var globale, iar la final le salvez la user
{
    i.oneYearHistory = oneYearHistory;
    i.d = d;
    i.m = m;
    i.y = y;
    i.tartrumHistory = tartrumHistory;
    i.minimBrushings = minimBrushings;
    i.maximBrushings = maximBrushings;
}

void addDay()
{
    vector<brushingData> brushings;
    oneYearHistory.push_back(brushings);
}

void startCronometru()
{
    start = high_resolution_clock::now();
    time_t now = time(0);
    tm *ltm = localtime(&now);
    nd = ltm->tm_mday;
    nm = 1 + ltm->tm_mon;
    ny = 1900 + ltm->tm_year;
}

void stopCronometru()
{
    finish = high_resolution_clock::now();
}

brushingData p;
void saveBrushingData()
{

    p.time = duration_cast<seconds>(finish - start).count();

    if (ny > y)
    {
        addDay();
    }
    else if (nm > m)
    {
        addDay();
    }
    else if (nd > d)
    {
        addDay();
    }
    d = nd;
    m = nm;
    y = ny;
    oneYearHistory[oneYearHistory.size() - 1].push_back(p); //adaug periaj in ziua curenta
    if (oneYearHistory.size() > 365)
    {
        oneYearHistory.erase(oneYearHistory.begin()); // sterg cea mai veche zi
    }
}

string statistics()
{
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
            brushingData p = oneYearHistory[i][j];
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

    vector<int> t = oneYearHistory.back().back().tartrum;
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

    string returnStatistics;
    returnStatistics += "Statistics \n";
    returnStatistics += "Last month: \n brushing time = ";
    returnStatistics += timeMonth;
    returnStatistics += "\n brushings with bleeding = ";
    returnStatistics += bleedingMonth + "% \n";
    returnStatistics += "Last year: \n brushing time = ";
    returnStatistics += timeYear;
    returnStatistics += "\n brushings with bleeding = ";
    returnStatistics += bleedingYear;
    returnStatistics += "% \n";
    returnStatistics += "To get rid of tartrum you need between ";
    returnStatistics += minimBrushings;
    returnStatistics += " and ";
    returnStatistics += maximBrushings;
    returnStatistics += "\n";

    return returnStatistics;
}

//------------------------------------------------------------


int getLedsColorFromString(string& s)
{
    /*
	Grey = 1,
	Blue,
	Green,
	Red
	*/
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
    /*
	No_Direction = 1,
	Right,
	Left,
	Turn_Off,
	Warning_Wrong_Direction
	*/
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
void getDirections(const Rest::Request &request, Http::ResponseWriter response)
{
    std::string returnString = "No brushing available!";
    unsigned j = 0;
    for (Brushing* &currBrushing : saved_Brushing)
    {
        returnString = "";
        for (unsigned i = 0; i < currBrushing->Directions.size(); ++i)
        {
            returnString += "Color is ";
            returnString += currBrushing->Leds[i];
            returnString += ", direction is ";
            returnString += currBrushing->Directions[i];
            returnString += currBrushing->Graphic_Directions[i];
            returnString += '\n';
            returnString += "================================";
        }
        returnString += currBrushing->History[j];
        returnString += '\n';
        returnString += "======================================= \n";
    }

    response.send(Http::Code::Ok, returnString.c_str());
}

std::string generateJSONTartrumObject(int index, float intensity, bool hasTartrum)
{
    std::string output = "\n\t{";
    output += "\n\t\t'toothNumber': " + std::to_string(index) + ",\n\t\t'intensity': '" + std::to_string(intensity) + "%',\n\t\t'health': '" + (hasTartrum ? "Tartrum'" : "Healthy'");
    output += "\n\t}";
    if (index < 32)
    {
        output += ",";
    }
    return output;
}

std::string checkParams(int index, int intensity)
{
    if (index > 32)
        return "Invalid parameters";
    if (intensity < 0 || intensity > 256)
        return "Intensity must be between 0 and 256";
    else
        return "VALID";
}

void getTeethCheck(const Rest::Request &request, Http::ResponseWriter response)
{
    std::string returnString = "[";
    auto TextParam = request.param(":statsArray").as<std::string>();
    std::cout << "Input Received : " << TextParam.c_str() << '\n';
    char *auxText = new char[strlen(TextParam.c_str())];
    strcpy(auxText, TextParam.c_str());
    char *token = strtok(auxText, ";");
    float tartrumThreshold = 60;
    int index = 1;

    while (token != NULL)
    {
        int intensity = atoi(token);
        float intensityPercentage = intensity * 100 / 256;
        // Check if the color intensity exceeds threshold
        bool hasTartrum = intensityPercentage > tartrumThreshold;

        // Check parameters sent to request
        std::string paramsError = checkParams(index, intensity);
        if (paramsError != "VALID")
        {
            response.send(Http::Code::Bad_Request, paramsError.c_str());
            return;
        }

        returnString += generateJSONTartrumObject(index, intensityPercentage, hasTartrum);

        if(hasTartrum)
        {
			p.tartrum.push_back(index-1);
		}

        ++index;
        token = strtok(NULL, ";");
    }

    returnString += "\n]";
    response.send(Http::Code::Ok, returnString.c_str());
}


void getGumBleeding(const Rest::Request &request, Http::ResponseWriter response)
{
    std::string returnString = "{\n";
    auto TextParam = request.param(":statsArray").as<std::string>();
    std::cout << "Input Received : " << TextParam.c_str() << '\n';
    char *auxText = new char[strlen(TextParam.c_str())];
    strcpy(auxText, TextParam.c_str());
    char *token = strtok(auxText, ";");
    float bleedingThreshold = 60;
    int index = 1;

    bool isBleeding = false;
    std::string bleedAreas = "[\n";

    while (token != NULL)
    {
        int intensity = atoi(token);
        float intensityPercentage = intensity * 100 / 256;
        // Check if the color intensity exceeds threshold
        bool currentToothBleed = intensityPercentage > bleedingThreshold;
        if (!isBleeding)
        {
            isBleeding = currentToothBleed;
        }
        // Check parameters sent to request
        std::string paramsError = checkParams(index, intensity);
        if (paramsError != "VALID")
        {
            response.send(Http::Code::Bad_Request, paramsError.c_str());
            return;
        }

        if (currentToothBleed)
        {
            if (bleedAreas.length() > 2)
                bleedAreas += ",\n";
            bleedAreas += "\t\t" + std::to_string(index);
        }

        ++index;
        token = strtok(NULL, ";");
    }

	p.bleeding = isBleeding; //adaugat de Albu, sper ca n-am stricat nimic :(

	string gumBleeding = isBleeding ? "true" : "false";
    returnString += "\t'hasBleeding' : " + gumBleeding;
    returnString += ",\n\t'areas' : " + bleedAreas;
    returnString += "\n\t]\n}";

    response.send(Http::Code::Ok, returnString.c_str());
}

int main() {
    const std::string clientId = "smarteeth";

    // Create a client
    mqtt::client client("localhost", clientId);

    mqtt::connect_options options;
    options.set_keep_alive_interval(20);
    options.set_clean_session(true);

    try {
        // Connect to the client
        client.connect(options);

        // Create a message
        const std::string TOPIC = "toothbrush";
        const std::string PAYLOAD = "Starting";
        auto msg = mqtt::make_message(TOPIC, PAYLOAD);

        // Publish it to the server
        client.publish(msg);

        // Disconnect
        client.disconnect();
    }
    catch (const mqtt::exception& exc) {
        std::cerr << exc.what() << " [" << exc.get_reason_code() << "]" << std::endl;
    }

    // Set up routes
    Router router;

    router.get("/hello", Routes::bind(helloRoute));
    //----------------------------------------------------------------------
    /*
    Usage :

    POST /configure
    {
        "name": "David",
        "age": 24,
        "program": 3,
        "teeth": [1,2,6,4,2,6,4,1,36,99,8]
    }

    GET /configure/David

    Tin sa mentionez faptul ca nu cred ca am abordat toate cazurile, erorile, s.a.m.d
    deci exista posibilitate sa revin si sa le dau update.

    */
    router.post("/configure", Routes::bind(setConfigure));
    router.get("/configure/:configurationName", Routes::bind(getConfigure));

    //  Usage :
    //  GET : localhost:9080/check/teeth/12;10;123;32;1;23;4;234;123;32;1;23;4;234;123;32;1;23;4;32;1;23;4;234;123;32;1;23;4;23;4;4
    router.get("/check/teeth/:statsArray", Routes::bind(getTeethCheck));
    //  GET : localhost:9080/check/gums/12;10;123;32;1;23;4;234;123;32;1;23;4;234;123;32;1;23;4;32;1;23;4;234;123;32;1;23;4;23;4;4
    router.get("/check/gums/:statsArray", Routes::bind(getGumBleeding));
    //----------------------------------------------------------------------
    // Configure server
    const string host = "localhost";
    const Port port = 9080;

    Address address(host, port);
    Endpoint endpoint(address);
    endpoint.init();
    endpoint.setHandler(router.handler());

    // Start server
    cout << "Server listening on http://" << host << ':' << port << '\n';
    cout << "Press Ctrl + C to quit\n";
    endpoint.serve();
}
