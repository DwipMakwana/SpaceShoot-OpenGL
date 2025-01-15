#include "stdafx.h"
#include "Galaxy.h"
#include <GL/gl.h>
#include <math.h>
#include <App/AppSettings.h>
#include <DebugUtils.h>

Galaxy::Galaxy(Renderer3D* renderer, int starsPerChunk, int numPlanets)
    : starsPerChunk(starsPerChunk),
    chunkSize(100.0f),
    renderer(renderer),
    numPlanets(numPlanets)
{
    const float MIN_PLANET_DISTANCE = 150.0f;  // Increased from 100.0f
    const float SPAWN_RANGE = 300.0f;  // Increased spawn range

    planets.resize(numPlanets);
    for (auto& planet : planets) {
        bool validPosition = false;
        while (!validPosition) {
            // Increased range for spawning
            planet.x = RandomRange(-SPAWN_RANGE, SPAWN_RANGE);
            planet.y = RandomRange(-SPAWN_RANGE, SPAWN_RANGE);
            planet.z = RandomRange(-SPAWN_RANGE, SPAWN_RANGE);

            validPosition = true;
            for (const auto& otherPlanet : planets) {
                if (&planet == &otherPlanet) continue;

                float distance = sqrt(pow(planet.x - otherPlanet.x, 2) +
                    pow(planet.y - otherPlanet.y, 2) +
                    pow(planet.z - otherPlanet.z, 2));

                if (distance < MIN_PLANET_DISTANCE) {
                    validPosition = false;
                    break;
                }
            }
        }

        int numRings = rand() % 5 + 2;

        for (int i = 0; i < numRings; i++) {
            Ring ring;
            ring.orbitRadius = RandomRange(15.0f, 25.0f); // Increment orbit radius for each ring
            ring.angle = RandomRange(0.0f, 360.0f) + (i * 45.0f);
            while (ring.angle >= 360.0f) ring.angle -= 360.0f;          // Random starting angle
            ring.selfAngle = 0.0f;                           // Initial self rotation
            ring.rotationSpeed = 10.0f;  // Slower orbit speed
            ring.selfRotationSpeed = RandomRange(5.0f, 15.0f); // Even slower self rotation
            planet.rings.push_back(ring);
        }
    }
}

ChunkKey Galaxy::GetChunkFromPosition(float x, float y, float z) {
    ChunkKey key;
    key.x = static_cast<int>(floor(x / chunkSize));
    key.y = static_cast<int>(floor(y / chunkSize));
    key.z = static_cast<int>(floor(z / chunkSize));
    return key;
}

void Galaxy::CreateStar(Star& star, const ChunkKey& chunk) {
    float minX = chunk.x * chunkSize;
    float minY = chunk.y * chunkSize;
    float minZ = chunk.z * chunkSize;

    star.x = RandomRange(minX, minX + chunkSize);
    star.y = RandomRange(minY, minY + chunkSize);
    star.z = RandomRange(minZ, minZ + chunkSize);

    // Use chunk coordinates as seed for consistent star generation
    unsigned int seed = chunk.x * 73856093 + chunk.y * 19349663 + chunk.z * 83492791;
    srand(seed);

    star.brightness = pow(Random(), 2.0f);

    // Size distribution
    float sizeRand = Random();
    if (sizeRand > 0.99f) {
        star.size = RandomRange(2.0f, 3.0f);
    }
    else if (sizeRand > 0.95f) {
        star.size = RandomRange(1.0f, 2.0f);
    }
    else {
        star.size = RandomRange(0.1f, 1.0f);
    }

    // Color distribution
    float colorType = Random();
    if (colorType > 0.95f) {  // Red giants
        star.r = RandomRange(0.8f, 1.0f);
        star.g = RandomRange(0.0f, 0.3f);
        star.b = RandomRange(0.0f, 0.2f);
    }
    else if (colorType > 0.90f) {  // Blue stars
        star.r = RandomRange(0.0f, 0.4f);
        star.g = RandomRange(0.0f, 0.4f);
        star.b = RandomRange(0.8f, 1.0f);
    }
    else {  // White/yellow stars
        float baseColor = RandomRange(0.7f, 1.0f);
        star.r = baseColor;
        star.g = baseColor;
        star.b = RandomRange(baseColor, 1.0f);
    }
}

void Galaxy::CreateChunk(const ChunkKey& key) {
    std::vector<Star>& chunkStars = chunks[key];
    chunkStars.resize(starsPerChunk);

    for (Star& star : chunkStars) {
        CreateStar(star, key);
    }
}

void Galaxy::UpdateVisibleChunks(const Camera& camera) {
    float camX, camY, camZ;
    camera.GetPosition(camX, camY, camZ);
    ChunkKey centerChunk = GetChunkFromPosition(camX, camY, camZ);

    const int RENDER_DISTANCE = 5;

    // Remove far chunks
    for (auto it = chunks.begin(); it != chunks.end();) {
        const ChunkKey& key = it->first;
        if (abs(key.x - centerChunk.x) > RENDER_DISTANCE ||
            abs(key.y - centerChunk.y) > RENDER_DISTANCE ||
            abs(key.z - centerChunk.z) > RENDER_DISTANCE) {
            it = chunks.erase(it);
        }
        else {
            ++it;
        }
    }

    // Create new chunks in range
    for (int x = centerChunk.x - RENDER_DISTANCE; x <= centerChunk.x + RENDER_DISTANCE; x++) {
        for (int y = centerChunk.y - RENDER_DISTANCE; y <= centerChunk.y + RENDER_DISTANCE; y++) {
            for (int z = centerChunk.z - RENDER_DISTANCE; z <= centerChunk.z + RENDER_DISTANCE; z++) {
                ChunkKey key{ x, y, z };
                if (chunks.find(key) == chunks.end()) {
                    CreateChunk(key);
                }
            }
        }
    }
}

void Galaxy::Update(float deltaTime, const Camera& camera) {
    float currentTime = (float)GetTickCount64() / 1000.0f;
    deltaTime = currentTime - previousTime;
    previousTime = currentTime;

    // Update explosions
    for (auto& explosion : explosions) {
        explosion.Update(deltaTime);
    }

    // Remove finished explosions
    explosions.erase(
        std::remove_if(explosions.begin(), explosions.end(),
            [](const ExplosionEffect& e) { return !e.IsActive(); }),
        explosions.end());

    UpdateVisibleChunks(camera);

    if (spaceship != nullptr) {
        float spaceshipX, spaceshipY, spaceshipZ;
        spaceship->GetPosition(spaceshipX, spaceshipY, spaceshipZ);

        char debugText[256];
        int closestRingIndex = -1;
        float closestDistance = FLT_MAX;

        // Process bullets
        for (auto bulletIt = bullets.begin(); bulletIt != bullets.end();) {
            bool bulletDestroyed = false;

            // Check collision with spaceship
            if (bulletIt->CheckSpaceshipCollision(spaceshipX, spaceshipY)) {
                spaceship->TakeDamage(Bullet::DAMAGE);
                bulletDestroyed = true;
            }

            if (bulletDestroyed) {
                bulletIt = bullets.erase(bulletIt);
            }
            else {
                ++bulletIt;
            }
        }

        for (int p = 0; p < planets.size(); p++) {
            auto& planet = planets[p];

            // Check if planet should become collectable
            if (!planet.isCollectable && !planet.HasActiveRings()) {
                planet.isCollectable = true;
            }

            // Check for collection if planet is collectable and not already collected
            if (planet.isCollectable && !planet.isCollected) {
                float dx = spaceshipX - planet.x;
                float dy = spaceshipY - planet.y;
                float distance = sqrt(dx * dx + dy * dy);

                if (distance < Planet::COLLECTION_RADIUS) {
                    // Collect the planet
                    planet.isCollected = true;

                    // Replenish spaceship health and ammo
                    spaceship->health = Spaceship::INITIAL_HEALTH;
                    spaceship->currentAmmo = Spaceship::MAX_AMMO;
                }
            }

            for (int r = 0; r < planet.rings.size(); r++) {
                auto& ring = planet.rings[r];

                if (!ring.isActive) continue;  // Skip inactive rings
                
                float normalizedAngle = fmodf(ring.angle, 360.0f);
                float angleInRadians = normalizedAngle * 3.14159f / 180.0f;

                // Calculate ring's world position
                float ringX = planet.x + ring.orbitRadius * cos(angleInRadians);
                float ringY = planet.y + ring.orbitRadius * sin(angleInRadians);
                float ringZ = planet.z;

                // Check spaceship's bullets against this ring
                std::vector<Bullet>& shipBullets = spaceship->GetBullets();
                for (auto& shipBullet : shipBullets) {
                    if (shipBullet.CheckRingCollision(ringX, ringY)) {
                        ring.health -= shipBullet.DAMAGE;

                        if (ring.health <= 0) {
                            explosions.emplace_back(ringX, ringY, 0);
                            ring.isActive = false;
                        }
                        shipBullet.Deactivate();
                    }
                }

                float dx = spaceshipX - ringX;
                float dy = spaceshipY - ringY;
                float distanceToShip = sqrt(dx * dx + dy * dy);

                if (distanceToShip < INFLUENCE_RADIUS && spaceship->IsAlive()) {
                    // Face the spaceship and stop orbital movement
                    float targetAngle = atan2f(dy, dx) * 180.0f / 3.14159f;
                    float influence = 1.0f - (distanceToShip / INFLUENCE_RADIUS);
                    ring.yawAngle = targetAngle;

                    fireTimer -= deltaTime;
                    if (fireTimer <= 0.0f) {
                        // Calculate firing position at the ring's current position
                        float spawnX = ringX;
                        float spawnY = ringY;

                        // Calculate angle towards spaceship
                        float angleToShip = atan2f(spaceshipY - ringY, spaceshipX - ringX) * 180.0f / 3.14159f;

                        // Fire bullet
                        FireBullet(spawnX, spawnY);
                        fireTimer = FIRE_RATE;  // Reset timer
                    }
                }
                else {
                    // Resume both orbital and self rotation when not influenced
                    ring.angle += ring.rotationSpeed * deltaTime;
                    if (ring.angle > 360.0f) ring.angle -= 360.0f;

                    ring.selfAngle += ring.selfRotationSpeed * deltaTime;
                    if (ring.selfAngle > 360.0f) ring.selfAngle -= 360.0f;

                    ring.yawAngle = 0.0f;
                }

                // Update bullets
                for (auto& bullet : bullets) {
                    bullet.Update(deltaTime, ringX, ringY);
                }
            }
        }
    }

    // Update bullets
    bullets.erase(
        std::remove_if(bullets.begin(), bullets.end(),
            [](const Bullet& b) { return !b.IsActive(); }),
        bullets.end());

    // Update star twinkling
    for (auto& pair : chunks) {
        std::vector<Star>& stars = pair.second;
        for (Star& star : stars) {
            if (Random() < 0.01f) {  // Only 1% of stars twinkle each frame
                star.brightness *= RandomRange(0.5f, 1.5f);
                if (star.brightness < 0.1f) star.brightness = 0.1f;
                if (star.brightness > 1.0f) star.brightness = 1.0f;
            }
        }
    }
}

void Galaxy::FireBullet(float spawnX, float spawnY) {
    // Calculate angle towards spaceship
    float spaceshipX, spaceshipY, spaceshipZ;
    spaceship->GetPosition(spaceshipX, spaceshipY, spaceshipZ);
    float angleToShip = atan2f(spaceshipY - spawnY, spaceshipX - spawnX) * 180.0f / 3.14159f;

    bullets.emplace_back(spawnX, spawnY, angleToShip, false);
}

float CalculateDistance(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrt(dx * dx + dy * dy);
}

void Normalize(float v[3]) {
    float len = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    v[0] /= len;
    v[1] /= len;
    v[2] /= len;
}

void Galaxy::DrawRings() {
    for (auto& planet : planets) {
        for (auto& ring : planet.rings) {
            if (!ring.isActive) continue;

            glPushMatrix();

            // Move to planet position and apply orbit rotation
            glTranslatef(planet.x, planet.y, 0.0f);

            glRotatef(ring.angle, 0.0f, 0.0f, 1.0f);            // Orbit rotation
            glTranslatef(ring.orbitRadius, 0.0f, 0.0f);         // Move to orbit position

            // Apply ring orientation
            glRotatef(ring.yawAngle, 1.0f, 1.0f, 0.0f);

            // Finally apply self rotation
            glRotatef(ring.selfAngle, 0.0f, 1.0f, 0.0f);

            // Set red color
            glColor3f(1.0f, 0.0f, 0.0f);

            // Draw torus (donut shape)
            const float ringRadius = 3.0f;        // Major radius (size of the ring)
            const float tubeRadius = 1.0f;        // Minor radius (thickness of the tube)
            const int ringSegments = 16;          // Segments around the ring
            const int tubeSegments = 8;          // Segments around the tube

            // Draw circles around the torus tube
            for (int i = 0; i < ringSegments; i++) {
                float phi = i * 2.0f * 3.14159f / ringSegments;
                float nextPhi = (i + 1) * 2.0f * 3.14159f / ringSegments;

                glBegin(GL_LINE_LOOP);
                for (int j = 0; j < tubeSegments; j++) {
                    float theta = j * 2.0f * 3.14159f / tubeSegments;

                    // Current point
                    float x = (ringRadius + tubeRadius * cosf(theta)) * cosf(phi);
                    float y = (ringRadius + tubeRadius * cosf(theta)) * sinf(phi);
                    float z = tubeRadius * sinf(theta);
                    glVertex3f(x, y, z);

                    // Next point
                    x = (ringRadius + tubeRadius * cosf(theta)) * cosf(nextPhi);
                    y = (ringRadius + tubeRadius * cosf(theta)) * sinf(nextPhi);
                    z = tubeRadius * sinf(theta);
                    glVertex3f(x, y, z);
                }
                glEnd();
            }

            // Draw square plane in the center of the ring
            float squareSize = 0.5f; // adjust this value to change the size
            glPushMatrix();
            glTranslatef(0.0f, 0.0f, 0.0f); // center of the ring
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // enable filled polygons
            glColor3f(1.0f, 0.0f, 0.0f); // red color
            glBegin(GL_QUADS);
            glVertex3f(-ringRadius * squareSize, -ringRadius * squareSize, 0.0f);
            glVertex3f(ringRadius * squareSize, -ringRadius * squareSize, 0.0f);
            glVertex3f(ringRadius * squareSize, ringRadius * squareSize, 0.0f);
            glVertex3f(-ringRadius * squareSize, ringRadius * squareSize, 0.0f);
            glEnd();
            glPopMatrix();

            glPopMatrix();
        }
    }
}

void Galaxy::DrawStars()
{
    // Draw stars
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glEnable(GL_POINT_SMOOTH);

    for (const auto& pair : chunks) {
        const std::vector<Star>& stars = pair.second;
        glBegin(GL_POINTS);
        for (const auto& star : stars) {
            glColor4f(star.r, star.g, star.b, star.brightness);
            glVertex3f(star.x, star.y, star.z);
        }
        glEnd();
    }
}

void Galaxy::DrawPlanets()
{
    for (const auto& planet : planets)
    {
        if (planet.isCollected) continue;

        // Draw central cube
        glPushMatrix();
        //glRotatef(1.0f * GetTickCount64() / 100.0f, 0.5f, -1.0f, 0.0f);
        glTranslatef(planet.x, planet.y, 0.0f);

        glDisable(GL_LIGHTING);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColor4f(1.0f, 0.0f, 1.0f, 1.0f);

        float size = 2.0f;
        glBegin(GL_QUADS);
        glVertex3f(-size, -size, size);
        glVertex3f(size, -size, size);
        glVertex3f(size, size, size);
        glVertex3f(-size, size, size);

        glVertex3f(-size, -size, -size);
        glVertex3f(-size, size, -size);
        glVertex3f(size, size, -size);
        glVertex3f(size, -size, -size);

        glVertex3f(-size, size, -size);
        glVertex3f(-size, size, size);
        glVertex3f(size, size, size);
        glVertex3f(size, size, -size);

        glVertex3f(-size, -size, -size);
        glVertex3f(size, -size, -size);
        glVertex3f(size, -size, size);
        glVertex3f(-size, -size, size);

        glVertex3f(size, -size, -size);
        glVertex3f(size, size, -size);
        glVertex3f(size, size, size);
        glVertex3f(size, -size, size);

        glVertex3f(-size, -size, -size);
        glVertex3f(-size, -size, size);
        glVertex3f(-size, size, size);
        glVertex3f(-size, size, -size);
        glEnd();
        glPopMatrix();

        // Only draw the green sphere if there are active rings
        if (planet.HasActiveRings()) {

            // Draw larger triangulated sphere
            glColor3f(0.0f, 1.0f, 0.0f);  // Green
            float sphereRadius = size * 4.0f;

            const float X = 0.525731112119133606f;
            const float Z = 0.850650808352039932f;
            const float N = 0.0f;

            float vdata[12][3] = {
                {-X, N, Z}, {X, N, Z}, {-X, N, -Z}, {X, N, -Z},
                {N, Z, X}, {N, Z, -X}, {N, -Z, X}, {N, -Z, -X},
                {Z, X, N}, {-Z, X, N}, {Z, -X, N}, {-Z, -X, N}
            };

            int tindices[20][3] = {
                {0,4,1}, {0,9,4}, {9,5,4}, {4,5,8}, {4,8,1},
                {8,10,1}, {8,3,10}, {5,3,8}, {5,2,3}, {2,7,3},
                {7,10,3}, {7,6,10}, {7,11,6}, {11,0,6}, {0,1,6},
                {6,1,10}, {9,0,11}, {9,11,2}, {9,2,5}, {7,2,11}
            };

            glPushMatrix();
            //glRotatef(2.0f * GetTickCount64() / 100.0f, 0.0f, 1.0f, 0.0f);
            glTranslatef(planet.x, planet.y, 0.0f);
            glBegin(GL_LINES);

            for (int i = 0; i < 20; i++) {
                // Get triangle vertices
                float v1[3] = { vdata[tindices[i][0]][0], vdata[tindices[i][0]][1], vdata[tindices[i][0]][2] };
                float v2[3] = { vdata[tindices[i][1]][0], vdata[tindices[i][1]][1], vdata[tindices[i][1]][2] };
                float v3[3] = { vdata[tindices[i][2]][0], vdata[tindices[i][2]][1], vdata[tindices[i][2]][2] };

                // Subdivide each triangle into 4
                float v12[3] = { (v1[0] + v2[0]) / 2, (v1[1] + v2[1]) / 2, (v1[2] + v2[2]) / 2 };
                float v23[3] = { (v2[0] + v3[0]) / 2, (v2[1] + v3[1]) / 2, (v2[2] + v3[2]) / 2 };
                float v31[3] = { (v3[0] + v1[0]) / 2, (v3[1] + v1[1]) / 2, (v3[2] + v1[2]) / 2 };

                Normalize(v12);
                Normalize(v23);
                Normalize(v31);

                // Draw original edges
                glVertex3f(v1[0] * sphereRadius, v1[1] * sphereRadius, v1[2] * sphereRadius);
                glVertex3f(v2[0] * sphereRadius, v2[1] * sphereRadius, v2[2] * sphereRadius);

                glVertex3f(v2[0] * sphereRadius, v2[1] * sphereRadius, v2[2] * sphereRadius);
                glVertex3f(v3[0] * sphereRadius, v3[1] * sphereRadius, v3[2] * sphereRadius);

                glVertex3f(v3[0] * sphereRadius, v3[1] * sphereRadius, v3[2] * sphereRadius);
                glVertex3f(v1[0] * sphereRadius, v1[1] * sphereRadius, v1[2] * sphereRadius);

                // Draw subdivided edges
                glVertex3f(v12[0] * sphereRadius, v12[1] * sphereRadius, v12[2] * sphereRadius);
                glVertex3f(v23[0] * sphereRadius, v23[1] * sphereRadius, v23[2] * sphereRadius);

                glVertex3f(v23[0] * sphereRadius, v23[1] * sphereRadius, v23[2] * sphereRadius);
                glVertex3f(v31[0] * sphereRadius, v31[1] * sphereRadius, v31[2] * sphereRadius);

                glVertex3f(v31[0] * sphereRadius, v31[1] * sphereRadius, v31[2] * sphereRadius);
                glVertex3f(v12[0] * sphereRadius, v12[1] * sphereRadius, v12[2] * sphereRadius);
            }
            glEnd();
            glPopMatrix();
        }
    }
}

Galaxy::ScreenPosition Galaxy::GetPlanetScreenPosition(const Planet& planet, const Camera& camera) {
    ScreenPosition result;

    // Save current matrices
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();

    // Set up scene matrices like we do for rendering
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float)APP_VIRTUAL_WIDTH / (float)APP_VIRTUAL_HEIGHT, 0.1f, 10000.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Apply camera transformation manually
    float camX, camY, camZ;
    float rotX, rotY, rotZ;
    camera.GetPosition(camX, camY, camZ);
    camera.GetRotation(rotX, rotY, rotZ);

    glRotatef(-rotX, 1.0f, 0.0f, 0.0f);
    glRotatef(-rotY, 0.0f, 1.0f, 0.0f);
    glRotatef(-rotZ, 0.0f, 0.0f, 1.0f);
    glTranslatef(-camX, -camY, -camZ);

    // Get matrices
    GLdouble modelview[16];
    GLdouble projection[16];
    GLint viewport[4];
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);

    GLdouble screenX, screenY, screenZ;
    gluProject(planet.x, planet.y, planet.z,
        modelview, projection, viewport,
        &screenX, &screenY, &screenZ);

    // Convert to virtual coordinates
    result.screenX = (screenX * APP_VIRTUAL_WIDTH) / viewport[2];
    result.screenY = ((viewport[3] - screenY) * APP_VIRTUAL_HEIGHT) / viewport[3];

    // Smaller margin and stricter bounds check
    float margin = 10.0f;  // Reduced margin
    result.isOnScreen = screenZ > 0 &&  // Check if in front of camera
        result.screenX >= margin &&
        result.screenX <= (APP_VIRTUAL_WIDTH - margin) &&
        result.screenY >= margin &&
        result.screenY <= (APP_VIRTUAL_HEIGHT - margin);

    //DebugPrint("Planet (%.2f, %.2f, %.2f) -> Screen (%.2f, %.2f) -> isOnScreen: %d screenZ: %.2f",
    //    planet.x, planet.y, planet.z,
    //    result.screenX, result.screenY,
    //    result.isOnScreen,
    //    screenZ);

    // Restore matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    return result;
}

void Galaxy::RenderDirectionArrows() {
    Camera& camera = renderer->GetCamera();

    // Save current matrices and set up 2D rendering
    glPushMatrix();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, APP_VIRTUAL_WIDTH, APP_VIRTUAL_HEIGHT, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Get camera info
    float camX, camY, camZ;
    float rotX, rotY, rotZ;
    camera.GetPosition(camX, camY, camZ);
    camera.GetRotation(rotX, rotY, rotZ);

    // Screen dimensions
    float screenWidth = APP_VIRTUAL_WIDTH;
    float screenHeight = APP_VIRTUAL_HEIGHT;
    float padding = 20.0f; // Padding from screen edge

    for (const auto& planet : planets) {
        ScreenPosition pos = GetPlanetScreenPosition(planet, camera);

        if (!pos.isOnScreen) {
            // Calculate direction to planet in world space
            float dirX = planet.x - camX;
            float dirY = planet.y - camY;

            // Convert to screen space considering camera rotation
            float screenAngle = atan2f(dirY, dirX) - (rotY * 3.14159f / 180.0f);

            // Calculate normalized direction vector
            float dx = cosf(screenAngle);
            float dy = -sinf(screenAngle);

            // Calculate arrow position on screen edge
            float arrowX, arrowY;
            float rotation;

            // Determine which screen edge to place the arrow on
            if (abs(dx) > abs(dy)) {
                // Place on left or right edge
                arrowX = (dx > 0) ? screenWidth - padding : padding;
                arrowY = screenHeight / 2 + (dy * screenWidth / 2) / abs(dx);
                // Clamp Y position
                arrowY = fmaxf(padding, fminf(screenHeight - padding, arrowY));
                rotation = (dx > 0) ? 0 : 180;
            }
            else {
                // Place on top or bottom edge
                arrowX = screenWidth / 2 + (dx * screenHeight / 2) / abs(dy);
                arrowY = (dy > 0) ? screenHeight - padding : padding;
                // Clamp X position
                arrowX = fmaxf(padding, fminf(screenWidth - padding, arrowX));
                rotation = (dy > 0) ? 90 : 270;
            }

            // Draw arrow
            glDisable(GL_LIGHTING);
            glPushMatrix();
            glTranslatef(arrowX, arrowY, 0);
            glRotatef(rotation, 0, 0, 1);

            glColor3f(1.0f, 1.0f, 1.0f);
            glLineWidth(2.0f);

            // Draw arrow shape
            glBegin(GL_LINE_LOOP);
            glVertex2f(-10, -5);
            glVertex2f(10, 0);
            glVertex2f(-10, 5);
            glEnd();

            glEnable(GL_LIGHTING);
            glPopMatrix();
        }
    }

    // Restore states
    glLineWidth(1.0f);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void Galaxy::Render() {
    glPushMatrix();
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    //Draw planets and stars
    DrawStars();
    DrawPlanets();
    DrawRings();

    glPopAttrib();
    glPopMatrix();

    // Render explosions
    for (auto& explosion : explosions) {
        explosion.Render();
    }

    // Render bullets
    for (auto& bullet : bullets) {
        bullet.Render(false);
    }

    // Render direction arrows for off-screen planets
    RenderDirectionArrows();
}