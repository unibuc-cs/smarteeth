#include <iostream>
#include <string>
#include <string.h>
#include <vector>
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
    std::string Name;
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
