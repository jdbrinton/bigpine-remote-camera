#include "random.h"
#include "mbed.h"

namespace aware
{

char RandomGenerator::sample_alphanumeric() {
    char c = sample(62);
    if (c < 10) {
        c += 48;
    } else if ( c < 36) {
        c += 55;
    } else {
        c += 61;
    }
    
}

bool SystemRandomGenerator::_initialized = false;

int SystemRandomGenerator::initialize(unsigned int seed)
{
    if (!_initialized)
    {
        srand(seed);
        return 0;
    }
    else
    {
        return -1;
    }
}

int SystemRandomGenerator::sample(int max)
{
    return rand() % max;
}

double SystemRandomGenerator::sample(double max)
{
    return (max / RAND_MAX) * rand();
}

AIRandomFactory::AIRandomFactory()
{
    if (!_rnd.initialized())
    {
        AnalogIn ai(MBED_CONF_APP_RAND_PIN);
        unsigned int seed = 0;
        for (int i = 0; i < 32; ++i)
        {
            unsigned int val = ai.read_u16();
            val = val >> 4;
            val &= 1;
            seed |= val << i;
        }
        _rnd.initialize(seed);
    }
}

RandomGenerator &AIRandomFactory::generator()
{
    return _rnd;
}
} // namespace aware
