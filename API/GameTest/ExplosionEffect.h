#ifndef EXPLOSION_EFFECT_H
#define EXPLOSION_EFFECT_H

#include <vector>
#include <windows.h>
#include <GL/gl.h>

struct Stick {
    float x, y, z;           // Position
    float vx, vy, vz;        // Velocity
    float rx, ry, rz;        // Rotation
    float vrotx, vroty, vrotz;  // Rotation velocity
    float length;            // Length of the stick
    float lifetime;          // How long this stick exists
    float alpha;             // Transparency
};

class ExplosionEffect {
public:
    ExplosionEffect(float x, float y, float z);
    void Update(float deltaTime);
    void Render(bool isSpaceship = false);
    bool IsActive() const { return !sticks.empty(); }

private:
    std::vector<Stick> sticks;
    static constexpr int NUM_STICKS = 10;           // Number of sticks to create
    static constexpr float MAX_VELOCITY = 20.0f;    // Maximum initial velocity
    static constexpr float MAX_ROT_VELOCITY = 360.0f; // Maximum rotation velocity
    static constexpr float STICK_LENGTH_MIN = 3.0f;  // Minimum stick length
    static constexpr float STICK_LENGTH_MAX = 7.0f;  // Maximum stick length
    static constexpr float MAX_LIFETIME = 2.0f;     // Maximum lifetime in seconds

    float Random() { return static_cast<float>(rand()) / RAND_MAX; }
    float RandomRange(float min, float max) { return min + Random() * (max - min); }
};

#endif