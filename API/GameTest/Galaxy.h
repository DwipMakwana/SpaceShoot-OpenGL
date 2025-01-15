#ifndef GALAXY_H
#define GALAXY_H

#include <vector>
#include <map>
#include "Renderer3D.h"
#include <Spaceship.h>
#include <UISystem.h>
#include <ExplosionEffect.h>

struct Star {
    float x, y, z;
    float brightness;
    float size;
    float r, g, b;
};

struct ChunkKey {
    int x, y, z;
    bool operator<(const ChunkKey& other) const {
        if (x != other.x) return x < other.x;
        if (y != other.y) return y < other.y;
        return z < other.z;
    }
};

struct Ring {
    static const int INITIAL_HEALTH = 10;

    float x, y;           // Position relative to planet
    float angle;          // Current orbit angle around planet
    float selfAngle;      // Ring's rotation around itself
    float yawAngle;       // Horizontal rotation to face target
    float pitchAngle;     // Vertical rotation to face target
    float orbitRadius;    // Distance from planet
    float rotationSpeed;  // Orbit rotation speed
    float selfRotationSpeed;  // Speed of rotation around itself
    int health = INITIAL_HEALTH;           // Add health for rings
    bool isActive = true;       // To track if ring is destroyed

    void TakeDamage(int amount) {
        health -= amount;
        if (health <= 0) {
            health = 0;
            isActive = false;
        }
    }

    int GetHealth() const { return health; }
    bool IsAlive() const { return health > 0; }
};

struct Planet {
    float x, y, z;
    std::vector<Ring> rings;  // Multiple rings per planet
    float orbitRadius;

    bool isCollectable = false;  // Becomes true when all rings are destroyed
    bool isCollected = false;    // Becomes true when picked up by spaceship

    bool HasActiveRings() const {
        for (const auto& ring : rings) {
            if (ring.isActive) return true;
        }
        return false;
    }

    static constexpr float COLLECTION_RADIUS = 5.0f;  // Distance at which player can collect the cube
};

class Galaxy {
public:
    Galaxy(Renderer3D* renderer, int starsPerChunk = 200, int numPlanets = 10);
    void DrawStars();
    void DrawPlanets();
    void DrawRings();
    void Render();
    void Update(float deltaTime, const Camera& camera);
    void FireBullet(float spawnX, float spawnY);
    void SetSpaceship(Spaceship* ship) { spaceship = ship; }

private:
    std::vector<Bullet> bullets;

    std::map<ChunkKey, std::vector<Star>> chunks;
    int starsPerChunk;
    float chunkSize;
    Renderer3D* renderer;
    Spaceship* spaceship;
    int numPlanets;
    std::vector<Planet> planets;

    void CreateChunk(const ChunkKey& key);
    void CreateStar(Star& star, const ChunkKey& chunk);
    ChunkKey GetChunkFromPosition(float x, float y, float z);
    void UpdateVisibleChunks(const Camera& camera);
    float Random() { return (float)rand() / RAND_MAX; }
    float RandomRange(float min, float max) { return min + Random() * (max - min); }

    const int RINGS_PER_PLANET = 1;  // Number of rings per planet
    const float RING_ROTATION_SPEED = 5.0f;  // Adjust this value to control the rotation speed
    const float INFLUENCE_RADIUS = 30.0f;  // Adjust this value to change reaction distance

    const float FIRE_RATE = 0.1f;

    float fireTimer = 0.0f;

    float previousTime = 0.0f;
    float deltaTime = 0.0f;

    struct ScreenPosition {
        bool isOnScreen;
        float screenX, screenY;
        float angle;
    };

    ScreenPosition GetPlanetScreenPosition(const Planet& planet, const Camera& camera);
    void RenderDirectionArrows();

    std::vector<ExplosionEffect> explosions;  // Store active explosions
};

#endif