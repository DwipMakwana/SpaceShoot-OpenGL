#include "stdafx.h"
#include "Spaceship.h"
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>
#include <string>
#include <iostream>
#include <App/AppSettings.h>
#include <App/app.h>

Spaceship::Spaceship()
    : posX(0.0f), posY(0.0f), posZ(0.0f),
    rotX(0.0f), rotY(0.0f), rotZ(0.0f),
    velocityX(0.0f), velocityY(0.0f), velocityZ(0.0f),
    size(1.0f), fireTimer(0.0f) {
}

void Spaceship::Move(float dx, float dy) {
    // Apply acceleration to velocity
    velocityX += dx * ACCELERATION;
    velocityY += dy * ACCELERATION;

    // Calculate current speed
    float speed = sqrt(velocityX * velocityX + velocityY * velocityY);

    // Limit to max speed
    if (speed > MAX_SPEED) {
        float scale = MAX_SPEED / speed;
        velocityX *= scale;
        velocityY *= scale;
    }
}

void Spaceship::LookAt(float mouseX, float mouseY) {
    // Convert mouse coordinates to center-based coordinates
    float centerX = APP_VIRTUAL_WIDTH / 2.0f;
    float centerY = APP_VIRTUAL_HEIGHT / 2.0f;
    float dx = mouseX - centerX;
    float dy = mouseY - centerY;

    // Calculate angle and convert to degrees
    rotZ = atan2f(dy, dx) * (180.0f / 3.14159f) + 90.0f;
}

void Spaceship::TakeDamage(int amount)
{
    health -= amount;
    if (health <= 0) {
        health = 0;
        // Create multiple explosion effects for bigger impact
        for (int i = 0; i < 1; i++) {  // Keeping it to 1 for now
            explosions.emplace_back(posX, posY, 0.0f);
        }
        isAlive = false;
    }
}

void Spaceship::Update(float deltaTime) {
    // Update existing explosions
    for (auto& explosion : explosions) {
        explosion.Update(deltaTime);
    }

    // Remove finished explosions
    explosions.erase(
        std::remove_if(explosions.begin(), explosions.end(),
            [](const ExplosionEffect& e) { return !e.IsActive(); }),
        explosions.end());

    // Update bullets
    for (auto& bullet : bullets) {
        bullet.Update(deltaTime, posX, posY);
    }

    // Remove inactive bullets
    bullets.erase(
        std::remove_if(bullets.begin(), bullets.end(),
            [](const Bullet& b) { return !b.IsActive(); }),
        bullets.end());

    if (!isAlive) return;

    // Apply velocity to position
    posX += velocityX * deltaTime;
    posY += velocityY * deltaTime;

    // Apply deceleration
    velocityX *= DECELERATION;
    velocityY *= DECELERATION;

    // Stop completely if moving very slowly
    if (fabs(velocityX) < 0.01f) velocityX = 0.0f;
    if (fabs(velocityY) < 0.01f) velocityY = 0.0f;

    // Update fire timer
    fireTimer -= deltaTime;
    
    // Check for firing
    if (App::IsKeyPressed(VK_LBUTTON) && fireTimer <= 0.0f) {
        FireBullet();
        fireTimer = FIRE_RATE;
    }
}

void Spaceship::FireBullet() {
    if (currentAmmo <= 0) return;
    
    // Decrease ammo count
    currentAmmo--;

    // Calculate spawn position
    float spawnX = posX;
    float spawnY = posY;
    bullets.emplace_back(spawnX, spawnY, rotZ - 90.0f);
}

void Spaceship::Render() {
    // Render explosions
    for (auto& explosion : explosions) {
        explosion.Render(true);
    }

    // Render bullets
    for (auto& bullet : bullets) {
        bullet.Render(true);
    }

    if (!isAlive) return;

    glPushMatrix();
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glTranslatef(posX, posY, posZ);
    glRotatef(rotZ, 0.0f, 0.0f, 1.0f);

    // Set purple color
    glColor4f(1.0f, 0.0f, 1.0f, 1.0f); // Purple

    // Enable wireframe mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Disable lighting
    glDisable(GL_LIGHTING);

    // Draw cone
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < 12; i++) {
        float angle = i * 10.0f;
        float x = 2.0f * cos(angle);
        float z = 2.0f * sin(angle);

        // Draw cone's body
        glVertex3f(x, 0.0f, z);
        glVertex3f(0.0f, -4.0f, 0.0f); // Apex of the cone
        glVertex3f(2.0f * cos(angle + 10.0f), 0.0f, 2.0f * sin(angle + 10.0f));
    }

    for (int i = 0; i < 36; i++) {
        float angle = i * 10.0f;
        float x = 2.0f * cos(angle);
        float z = 2.0f * sin(angle);

        // Draw cone's base
        //glVertex3f(x, 0.0f, z);
        //glVertex3f(2.0f * cos(angle + 10.0f), 0.0f, 2.0f * sin(angle + 10.0f));
        //glVertex3f(0.0f, 0.0f, 0.0f); // Center of the base
    }

    glEnd();

    glPopAttrib();
    glPopMatrix();
}