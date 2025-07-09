#include <random>

class RandomNumberGenerator 
{
public:
    RandomNumberGenerator() : dis(0, 1) 
    {
        // Create a random device
        std::random_device rd;

        // Initialize Mersenne Twister pseudo-random number generator
        gen = std::mt19937(rd());
    }

    // Generate a random number
    float rand() 
    {
        return dis(gen);
    }

private:
    // Mersenne Twister pseudo-random number generator
    std::mt19937 gen;
    // Uniform real distribution between 0 and 1
    std::uniform_real_distribution<> dis;
};
