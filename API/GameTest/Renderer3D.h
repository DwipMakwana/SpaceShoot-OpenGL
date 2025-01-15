//------------------------------------------------------------------------
// Renderer3D.h
//------------------------------------------------------------------------
#ifndef RENDERER3D_H
#define RENDERER3D_H

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

class Camera {
public:
    Camera() :
        posX(0.0f), posY(0.0f), posZ(5.0f),
        rotX(0.0f), rotY(0.0f), rotZ(0.0f) {
    }

    void Move(float dx, float dy, float dz);
    void Rotate(float dx, float dy, float dz);
    void Apply();

    // Getter methods
    void GetPosition(float& x, float& y, float& z) const {
        x = posX;
        y = posY;
        z = posZ;
    }

    void GetRotation(float& x, float& y, float& z) const {
        x = rotX;
        y = rotY;
        z = rotZ;
    }

    // Setter methods
    void SetPosition(float x, float y, float z) {
        posX = x;
        posY = y;
        posZ = z;
    }

    void SetRotation(float x, float y, float z) {
        rotX = x;
        rotY = y;
        rotZ = z;
    }

    float posX, posY, posZ;    // Position
    float rotX, rotY, rotZ;    // Rotation angles
private:
};

class Renderer3D {
public:
    Renderer3D();
    void Initialize(int width, int height);
    void SetupScene();
    void DrawCube(float x, float y, float z, float size, float r, float g, float b);
    void DrawPyramid(float x, float y, float z, float size, float r, float g, float b);
    void DrawSphere(float x, float y, float z, float radius, float r, float g, float b);
    void UpdateLight(float x, float y, float z);
    Camera& GetCamera() { return camera; }

private:
    Camera camera;
    int screenWidth;
    int screenHeight;
};

#endif