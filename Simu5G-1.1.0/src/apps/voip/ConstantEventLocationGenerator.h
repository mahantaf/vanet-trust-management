#ifndef _CONSTANTEVENTLOCATIONGENERATOR_H_
#define _CONSTANTEVENTLOCATIONGENERATOR_H_

#include "inet/common/geometry/common/Coord.h"

#include <unordered_set>

#define EVIL_LOCATION_X 420
#define EVIL_LOCATION_Y 270

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

        // Added by Mahan
        bool dynamicLocation;
        inet::Coord eventLocation;

};

#endif /*_CONSTANTEVENTLOCATIONGENERATOR_H_*/
