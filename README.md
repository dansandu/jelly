# Jelly
Serialize, deserialize and use JSON objects in C++ code.
## Building and deploying the project
The project uses [Praline](https://github.com/dansandu/praline) as a build tool. Clone the project to a desired directory and run the following command inside the terminal:
```bash
praline.py deploy
```
This will build and deploy the jelly artifact to your Praline repository. Once deployed, the artifact can be used by any Praline project referencing the same Praline repository.
## Usage
The following code showcases the `Json` class: 
```cpp
#include "dansandu/jelly/json.hpp"

#include <iostream>

using dansandu::jelly::json::Json;

int main(int, char**)
{
    auto order = Json::deserialize(R"({
        "orderId": "471fc736-56e9-4a78-a256-4b6f641b7d13",
        "items": [
            {
                "itemId": "e01d9d5d-2994-4eeb-b24d-0c80fcfe7be0",
                "count": 5
            },
            {
                "itemId": "52cace0f-e482-4cf0-a5e5-0d8728e3015d",
                "count": 2
            }
        ],
        "hasPromotion": true,
        "vat": 0.20,
        "previousOrderId": null
    })");

    const std::string orderId          = order["orderId"];
    const int         firstItemCount   = order["items"][0]["count"];
    const int         secondItemCount  = order["items"][1]["count"].get<int>();
    const double      vat              = order["vat"];
    const bool        hasPreviousOrder = !order["previousOrderId"].is<std::nullptr_t>();

    // const std::nullptr_t    badCast1 = order["previousOrderId"];  // won't compile
    // const int*              badCast2 = order["previousOrderId"];  // won't compile
    // const unsigned          badCast3 = order["vat"];              // won't compile
    // const int               badCast4 = order["vat"];              // compiles but throws exception
    // const Json::list_type   badCast5 = order["items"];            // won't compile
    // const Json::object_type badCast6 = order;                     // won't compile

    auto total = (1.0 + vat) * (firstItemCount * 10.99 + secondItemCount * 55.99);

    if (order["hasPromotion"])
    {
        total *= 0.90;
    }

    auto orderReceipt = Json::object();
    orderReceipt["orderId"] = order["orderId"];
    orderReceipt["total"]   = total;

    std::cout << orderReceipt;
    return 0;
}
```
The program prints to the terminal the order receipt serialized as a JSON object.
```json
{"orderId":"471fc736-56e9-4a78-a256-4b6f641b7d13","total":180.284}
```
