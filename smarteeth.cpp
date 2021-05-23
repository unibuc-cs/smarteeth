#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <utility>
#include <pistache/endpoint.h>
#include <pistache/router.h>
#include <nlohmann/json.hpp>
#include <mqtt/client.h>

using namespace std;
using namespace Pistache;
using namespace Pistache::Http;
using namespace Pistache::Rest;

// Convenient namespace alias
using json = nlohmann::json;

class Toothbrush {
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
struct Config
{
    std::string Name;
    int age;
    eType program;
    std::vector<int> STeeth; // dintii
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

void helloRoute(const Rest::Request& request, Http::ResponseWriter response) {
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

    if(!foundVar)
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
    switch(p)
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
            if(!oConfig->STeeth.empty())
            {
                returnString += "These teeth are supervised : ";
                for(unsigned int i = 0 ; i < oConfig->STeeth.size(); ++i)
                    returnString += to_string(oConfig->STeeth[i]) + " ";
            }
        }
    response.send(Http::Code::Ok, returnString.c_str());
}

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
void setDirectionAndLedsGraphic(const Config &configuration)
{
    //Config configuration = getConfigure;
    //afla cum faci rost de configuratie din request direct

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
    } else
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
        //salvez culorile ledurilor si directia pe care o indica
        currentBrushing->Leds.push_back(Colors[i]);
        currentBrushing->Directions.push_back(getDirectionFromNumber(getLedsColorFromString(Colors[i])));

        //calculez ce inseamna directiile in functie de perierea care are loc
        if (getLedsColorFromString(Colors[i]) == 1)
        {
            if (!(std::count(verif.begin(), verif.end(), 0)))
            {
                returnString += "\n Completely brushing!";
            } else
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
                    returnString +=  " teeth. \n";
                    // nu marcam dintele care periaza ca fiind periat deoarce vom reveni asupra lui in caz ca va fi nevoie
                }
            } else
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
