#ifndef _CONSTANTEVENTLOCATIONGENERATOR_H_
#define _CONSTANTEVENTLOCATIONGENERATOR_H_

#include "inet/common/geometry/common/Coord.h"

#include <unordered_set>

#define EVIL_LOCATION_X 230
#define EVIL_LOCATION_Y 250 

class ConstantEventLocationGenerator
{   
    protected:
        std::unordered_set<std::string> evilCars;
    public:
        ConstantEventLocationGenerator(std::string evilCarsCsv);
        
        ConstantEventLocationGenerator()
        {

        }
        ~ConstantEventLocationGenerator()
        {

        }

        bool isEvilVehicle(std::string carID);
        inet::Coord getEventLocation(std::string carID);

};

#endif /*_CONSTANTEVENTLOCATIONGENERATOR_H_*/