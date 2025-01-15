#include "stdafx.h"
#include "ExplosionEffect.h"
#include <math.h>
#include <DebugUtils.h>

ExplosionEffect::ExplosionEffect(float x, float y, float z) {
    // Create initial sticks
    for (int i = 0; i < NUM_STICKS; i++) {
        Stick stick;
        stick.x = x;
        stick.y = y;
        stick.z = z;

        // Random velocity in all directions
        float angle = RandomRange(0, 2 * 3.14159f);
        float elevation = RandomRange(-3.14159f / 2, 3.14159f / 2);
        float speed = RandomRange(MAX_VELOCITY * 0.5f, MAX_VELOCITY);

        stick.vx = speed * cos(elevation) * cos(angle);
        stick.vy = speed * cos(elevation) * sin(angle);
        stick.vz = speed * sin(elevation);

        // Random rotation velocities
        stick.vrotx = RandomRange(-MAX_ROT_VELOCITY, MAX_ROT_VELOCITY);
        stick.vroty = RandomRange(-MAX_ROT_VELOCITY, MAX_ROT_VELOCITY);
        stick.vrotz = RandomRange(-MAX_ROT_VELOCITY, MAX_ROT_VELOCITY);

        // Initial rotation
        stick.rx = RandomRange(0, 360);
        stick.ry = RandomRange(0, 360);
        stick.rz = RandomRange(0, 360);

        stick.length = RandomRange(STICK_LENGTH_MIN, STICK_LENGTH_MAX);
        stick.lifetime = MAX_LIFETIME;
        stick.alpha = 1.0f;

        sticks.push_back(stick);
    }
}

void ExplosionEffect::Update(float deltaTime) {
    for (auto it = sticks.begin(); it != sticks.end();) {
        // Update position
        it->x += it->vx * deltaTime;
        it->y += it->vy * deltaTime;
        it->z += it->vz * deltaTime;

        // Update rotation
        it->rx += it->vrotx * deltaTime;
        it->ry += it->vroty * deltaTime;
        it->rz += it->vrotz * deltaTime;

        // Update lifetime and alpha
        it->lifetime -= deltaTime;
        it->alpha = it->lifetime / MAX_LIFETIME;

        // Remove stick if lifetime is over
        if (it->lifetime <= 0) {
            it = sticks.erase(it);
        }
        else {
            ++it;
        }
    }
}

void ExplosionEffect::Render(bool isSpaceship) {
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (const auto& stick : sticks) {
        glPushMatrix();
        glTranslatef(stick.x, stick.y, stick.z);
        glRotatef(stick.rx, 1, 0, 0);
        glRotatef(stick.ry, 0, 1, 0);
        glRotatef(stick.rz, 0, 0, 1);

        // Draw stick as a line
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        glColor4f(1.0f, 0.0f, isSpaceship ? 1.0f : 0.0f, stick.alpha);  // Red color with fading alpha
        glVertex3f(-stick.length / 2, 0, 0);
        glVertex3f(stick.length / 2, 0, 0);
        glEnd();

        glPopMatrix();
    }

    glPopAttrib();
}