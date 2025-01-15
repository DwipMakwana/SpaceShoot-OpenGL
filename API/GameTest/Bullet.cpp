#include "stdafx.h"
#include "Bullet.h"
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>

Bullet::Bullet(float startX, float startY, float angle, bool isSpaceshipBullet)
    : x(startX), y(startY), z(0.0f), angle(angle), active(true), fireTime(GetTickCount64())
{
    // Calculate velocity based on angle
    float radians = angle * 3.14159f / 180.0f;
    float speed = isSpaceshipBullet ? SPACESHIP_BULLET_SPEED : RING_BULLET_SPEED;
    velX = cos(radians) * speed;
    velY = sin(radians) * speed;
}

bool Bullet::CheckSpaceshipCollision(float shipX, float shipY, float collisionRadius) const {
    if (!active) return false;

    // Simple circle collision
    float dx = x - shipX;
    float dy = y - shipY;
    float distanceSquared = dx * dx + dy * dy;

    return distanceSquared < (collisionRadius * collisionRadius);
}

bool Bullet::CheckRingCollision(float ringX, float ringY, float collisionRadius) const {
    if (!active) return false;

    // Simple circle collision
    float dx = x - ringX;
    float dy = y - ringY;
    float distanceSquared = dx * dx + dy * dy;

    return distanceSquared < (collisionRadius * collisionRadius);
}

void Bullet::Update(float deltaTime, float spaceshipX, float spaceshipY) {
    if (!active) return;

    // Update position
    x += velX * deltaTime;
    y += velY * deltaTime;

    // Check if bullet has been active for too long
    if (GetTickCount64() - fireTime > BULLET_LIFETIME_MS) {
        active = false;
    }
}

void Bullet::Render(bool spaceship) {
    if (!active) return;

    glPushMatrix();
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glTranslatef(x, y, z);

    if (spaceship)
        glColor3f(1.0f, 1.0f, 1.0f);  // White color
    else
        glColor3f(1.0f, 0.0f, 0.0f);  // Red color

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDisable(GL_LIGHTING);

    // Draw bullet as small cubes
    float bulletSize = spaceship ? BULLET_SIZE : BULLET_RING_SIZE;

    // Front face
    glBegin(GL_LINE_LOOP);
    glVertex3f(-bulletSize, -bulletSize, bulletSize);
    glVertex3f(bulletSize, -bulletSize, bulletSize);
    glVertex3f(bulletSize, bulletSize, bulletSize);
    glVertex3f(-bulletSize, bulletSize, bulletSize);
    glEnd();

    // Back face
    glBegin(GL_LINE_LOOP);
    glVertex3f(-bulletSize, -bulletSize, -bulletSize);
    glVertex3f(-bulletSize, bulletSize, -bulletSize);
    glVertex3f(bulletSize, bulletSize, -bulletSize);
    glVertex3f(bulletSize, -bulletSize, -bulletSize);
    glEnd();

    // Top face
    glBegin(GL_LINE_LOOP);
    glVertex3f(-bulletSize, bulletSize, -bulletSize);
    glVertex3f(-bulletSize, bulletSize, bulletSize);
    glVertex3f(bulletSize, bulletSize, bulletSize);
    glVertex3f(bulletSize, bulletSize, -bulletSize);
    glEnd();

    // Bottom face
    glBegin(GL_LINE_LOOP);
    glVertex3f(-bulletSize, -bulletSize, -bulletSize);
    glVertex3f(bulletSize, -bulletSize, -bulletSize);
    glVertex3f(bulletSize, -bulletSize, bulletSize);
    glVertex3f(-bulletSize, -bulletSize, bulletSize);
    glEnd();

    // Right face
    glBegin(GL_LINE_LOOP);
    glVertex3f(bulletSize, -bulletSize, -bulletSize);
    glVertex3f(bulletSize, bulletSize, -bulletSize);
    glVertex3f(bulletSize, bulletSize, bulletSize);
    glVertex3f(bulletSize, -bulletSize, bulletSize);
    glEnd();

    // Left face
    glBegin(GL_LINE_LOOP);
    glVertex3f(-bulletSize, -bulletSize, -bulletSize);
    glVertex3f(-bulletSize, -bulletSize, bulletSize);
    glVertex3f(-bulletSize, bulletSize, bulletSize);
    glVertex3f(-bulletSize, bulletSize, -bulletSize);
    glEnd();

    glPopAttrib();
    glPopMatrix();
}
