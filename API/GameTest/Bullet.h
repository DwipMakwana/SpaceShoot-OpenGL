#include <cstdint>
#ifndef BULLET_H
#define BULLET_H

class Bullet {
public:
    static const int DAMAGE = 1;  // Each bullet damage

    Bullet(float startX, float startY, float angle, bool isSpaceshipBullet = true);
    void Update(float deltaTime, float spaceshipX, float spaceshipY);
    void Render(bool spaceship);
    bool IsActive() const { return active; }
    bool CheckSpaceshipCollision(float shipX, float shipY, float collisionRadius = 4.0f) const;
    bool CheckRingCollision(float ringX, float ringY, float collisionRadius = 3.0f) const;
    void Deactivate() { active = false; }

    // Getters for position (will be useful for collision detection)
    float GetX() const { return x; }
    float GetY() const { return y; }
    float GetZ() const { return z; }

private:
    float x, y, z;         // Position
    float velX, velY;      // Velocity
    float angle;           // Direction
    bool active;           // Is bullet active
    uint64_t fireTime;

    static constexpr float SPACESHIP_BULLET_SPEED = 50.0f;  // Speed for spaceship bullets
    static constexpr float RING_BULLET_SPEED = 2.5f;      // Speed for ring bullets
    static constexpr float BULLET_LIFETIME_MS = 1000.0f;  // For how much time bullet can travel
    static constexpr float BULLET_SIZE = 0.45f;      // Size of bullet
    static constexpr float BULLET_RING_SIZE = 0.9f;      // Size of bullet of the rings
};

#endif