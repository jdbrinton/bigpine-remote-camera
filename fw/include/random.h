#ifndef AWARE_RANDOM_H
#define AWARE_RANDOM_H

#include "mbed.h"

namespace aware
{

class RandomGenerator
{
  public:
    virtual int sample(int max) = 0;       // Range: [0, max[, max value excluded
    virtual double sample(double max) = 0; // Range: [0, max[, max value excluded
    char sample_alphanumeric(); // [0-9, a-z, A-Z]
};


class SystemRandomGenerator : public RandomGenerator
{
  public:
    int initialize(unsigned int seed = 0);
    int sample(int max) override;       // Range: [0, max[, max value excluded
    double sample(double max) override; // Range: [0, max[, max value excluded
    inline bool initialized() const { return _initialized; }

  private:
    static bool _initialized;
};

class RandomFactory
{
public:
    virtual RandomGenerator &generator() = 0;
};

class AIRandomFactory : public RandomFactory
{
  public:
    AIRandomFactory();
    RandomGenerator &generator() override;

  private:
    SystemRandomGenerator _rnd;
};

} // namespace aware

#endif