#include "apps/voip/ConstantEventLocationGenerator.h"

ConstantEventLocationGenerator::ConstantEventLocationGenerator(std::string evilCarsCsv)
{
    std::stringstream ss(evilCarsCsv);
 
    while (ss.good()) {
        std::string substr;
        getline(ss, substr, ',');
        evilCars.insert(substr);
    }
}

bool ConstantEventLocationGenerator::isEvilVehicle(std::string carID)
{
    if (evilCars.find(carID) == evilCars.end()) {
        return false;
    }
    return true;
}

inet::Coord ConstantEventLocationGenerator::getEventLocation(std::string carID)
{
    // Z coordinate is 0 since cars are moving in 2D plane only
    return inet::Coord(EVIL_LOCATION_X, EVIL_LOCATION_Y, 0);
}