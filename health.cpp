#include "health.hpp"

#include <stdexcept>

const static float TARTRUM_THRESHOLD = 40;
const static float BLEEDING_THRESHOLD = 70;

static unsigned char intensity;

void updateToothColor(int data)
{
    if (intensity < 0 || intensity > 256)
        throw new std::runtime_error("Intensity must be between 0 and 256");

    // VALID
    intensity = data;
}

ToothHealthInfo checkTartrum()
{
    float intensityPercentage = intensity * 100.0 / 256;

    // Check if the color intensity exceeds threshold
    bool hasTartrum = intensityPercentage > TARTRUM_THRESHOLD;

    if (hasTartrum)
    {
        // TODO: statistics
    }

    return ToothHealthInfo{intensity, hasTartrum};
}

ToothHealthInfo checkGumBleeding()
{
    float intensityPercentage = intensity * 100.0 / 256;

    // Check if the color intensity exceeds threshold
    bool isBleeding = intensityPercentage > BLEEDING_THRESHOLD;

    if (isBleeding)
    {
        // TODO: statistics
    }

    return ToothHealthInfo{intensity, isBleeding};
}
