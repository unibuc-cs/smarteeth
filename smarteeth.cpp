#include <iostream>
#include <string>

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

void helloRoute(const Rest::Request& request, Http::ResponseWriter response) {
    // Read the request data
    cout << "Received a request from IP address " << request.address().host() << '\n';

    // Send a reply
    response.send(Http::Code::Ok, "Hello world!");
}

int main() {
    // Set up routes
    Router router;

    router.get("/hello", Routes::bind(helloRoute));

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
