#include <Bullet.h>
#include <vector>
#include <ExplosionEffect.h>
#ifndef SPACESHIP_H
#define SPACESHIP_H

class Spaceship {
public:
    Spaceship();
    void Render();
    void Update(float deltaTime);
    void Move(float dx, float dy);
    void LookAt(float mouseX, float mouseY);  // New function for mouse look
    int GetAmmo() const { return currentAmmo; }

    void GetPosition(float& x, float& y, float& z) const {
        x = posX; y = posY; z = posZ;
    }

    void SetPosition(float x, float y, float z) {
        posX = x; posY = y; posZ = z;
    }

    bool IsAlive() const { return isAlive; }
    int GetHealth() const { return health; }
    void TakeDamage(int amount);

    const std::vector<Bullet>& GetBullets() const { return bullets; }
    std::vector<Bullet>& GetBullets() { return bullets; }

    int health = INITIAL_HEALTH;          // Add health for spaceship
    int currentAmmo = MAX_AMMO;

    static constexpr int INITIAL_HEALTH = 100;
    static constexpr int MAX_AMMO = 100;

    static constexpr float CROSSHAIR_RADIUS = 10.0f;         // Size of circle
    static constexpr float CROSSHAIR_LINE_LENGTH = 14.0f;    // Length of plus lines
    static constexpr float CROSSHAIR_THICKNESS = 2.0f;       // Thickness of both circle and plus
    static constexpr int CIRCLE_SEGMENTS = 32;               // Smoothness of circle

    static constexpr float FIRE_RATE = 0.065f;

private:
    float fireTimer;           // New: Timer for shooting
    std::vector<Bullet> bullets;
    void FireBullet();

    float posX, posY, posZ;    // Position
    float rotX, rotY, rotZ;    // Rotation
    float size;                // Size of the spaceship

    bool isAlive = true;       // Track if spaceship is destroyed

    float velocityX, velocityY, velocityZ;  // Current velocity
    static constexpr float MAX_SPEED = 100.0f;        // Maximum speed
    static constexpr float ACCELERATION = 4.0f;     // Acceleration rate
    static constexpr float DECELERATION = 0.98f;    // Deceleration factor

    std::vector<ExplosionEffect> explosions;
};

#endif