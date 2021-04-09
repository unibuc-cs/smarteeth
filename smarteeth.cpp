#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <pistache/endpoint.h>
#include <pistache/router.h>

using namespace std;
using namespace Pistache;
using namespace Pistache::Http;
using namespace Pistache::Rest;

class Toothbrush {
public:
    Toothbrush() {
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
 char* Name;
 int age;
 eType program;
 std::vector<int> STeeth;
};
std::vector<Config*> saved_Configs;
/*
Teeth Skema
maybe using enum in the near future and names to parse c1(canin 1) m1(molar 1) will see, will see
1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16
32 31 30 29 28 27 26 25 24 23 22 21 20 19 18  17
Source :
https://ibb.co/KhfB2Kp

da cuaie stiu ca am scris theets calmeaza-te nu urla

*/

void helloRoute(const Rest::Request& request, Http::ResponseWriter response) {
    // Read the request data
    cout << "Received a request from IP address " << request.address().host() << '\n';

    // Send a reply
    response.send(Http::Code::Ok, "Hello world!");
} 
void setConfigure(const Rest::Request& request, Http::ResponseWriter response)
{
    Config* ourConfiguration;
    auto TextParam = request.param(":configureJSON").as<std::string>();
    std::cout << "Input Received : " <<TextParam.c_str() << '\n';
    char* auxText = new char[strlen(TextParam.c_str())];
    strcpy(auxText,TextParam.c_str());
    char* token = strtok(auxText,";");
    int index = 1;
    bool foundVar = false;
    for(Config* &oC : saved_Configs)
        if(!strcmp(oC->Name,token))
        {
            foundVar = true;
            ourConfiguration = oC;
            ourConfiguration->STeeth.clear(); //ATENTIE!! Numele este unic, daca se primesc alte setari cu acelasi nume se considera update.
        }

    if(!foundVar)
    {
        ourConfiguration = new Config();
        saved_Configs.push_back(ourConfiguration);
    }

    while(token != NULL)
    {
        if(index == 1)
        { 
            ourConfiguration->Name = new char[strlen(token)];   
            strcpy(ourConfiguration->Name,token);
        }
        else if(index == 2)
        {
            ourConfiguration->age = atoi(token);
        }
        else if(index == 3)
        {
            eType ourType = (eType)atoi(token);
            ourConfiguration->program = ourType;
        }
        if(strstr(token,","))
        {
            char* aux_token = strtok(token, ",");
            while(aux_token != NULL)
            {
                int teeth = atoi(aux_token);
                if(teeth < 1 || teeth > 32 || std::find(ourConfiguration->STeeth.begin(),ourConfiguration->STeeth.end(),teeth) != ourConfiguration->STeeth.end())
                {
                    aux_token = strtok(NULL, ",");
                    continue;
                }
                ourConfiguration->STeeth.push_back(teeth);
                aux_token = strtok(NULL, ",");
            }
            break;
        }
        ++index;
        token = strtok(NULL, ";");
    }
    delete auxText;
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
        if(!strcmp(oConfig->Name,TextParam.c_str()))
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
int main() {
    // Set up routes
    Router router;

    router.get("/hello", Routes::bind(helloRoute));
    //----------------------------------------------------------------------
    /*
    Usage : 
    Post : localhost:9080/configure/David;24;3;1,2,6,4,2,6,4,1,36,99,8
    Get : localhost:9080/configure/David
    Tin sa mentionez faptul ca nu cred ca am abordat toate cazurile, erorile, s.a.m.d 
    deci exista posibilitate sa revin si sa le dau update.

    */
    router.post("/configure/:configureJSON", Routes::bind(setConfigure));
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
