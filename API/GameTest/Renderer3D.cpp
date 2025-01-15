//------------------------------------------------------------------------
// Renderer3D.cpp
//------------------------------------------------------------------------
#include "stdafx.h"
#include "Renderer3D.h"
#include <math.h>

void Camera::Move(float dx, float dy, float dz) {
    posX += dx;
    posY += dy;
    posZ += dz;
}

void Camera::Rotate(float dx, float dy, float dz) {
    rotX += dx;
    rotY += dy;
    rotZ += dz;
}

void Camera::Apply() {
    glRotatef(-rotX, 1.0f, 0.0f, 0.0f);
    glRotatef(-rotY, 0.0f, 1.0f, 0.0f);
    glRotatef(-rotZ, 0.0f, 0.0f, 1.0f);
    glTranslatef(-posX, -posY, -posZ);
}

Renderer3D::Renderer3D() : screenWidth(800), screenHeight(600) {
}

void Renderer3D::Initialize(int width, int height) {
    screenWidth = width;
    screenHeight = height;

    glEnable(GL_DEPTH_TEST);

    // Enable lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // Set up ambient light
    float ambientLight[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);

    // Set up the main light
    float lightPos[] = { 10.0f, 10.0f, 10.0f, 1.0f };
    float lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    // Enable material properties
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    // Set default material properties
    float materialSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float materialShininess[] = { 50.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);

    // Set up projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float)width / (float)height, 0.1f, 10000.0f);
    glMatrixMode(GL_MODELVIEW);
}

void Renderer3D::SetupScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    camera.Apply();
}

void Renderer3D::UpdateLight(float x, float y, float z) {
    float lightPos[] = { x, y, z, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
}

void Renderer3D::DrawCube(float x, float y, float z, float size, float r, float g, float b) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glColor3f(r, g, b);

    float halfSize = size * 0.5f;
    glBegin(GL_QUADS);

    // Front face
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-halfSize, -halfSize, halfSize);
    glVertex3f(halfSize, -halfSize, halfSize);
    glVertex3f(halfSize, halfSize, halfSize);
    glVertex3f(-halfSize, halfSize, halfSize);

    // Back face
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(-halfSize, -halfSize, -halfSize);
    glVertex3f(-halfSize, halfSize, -halfSize);
    glVertex3f(halfSize, halfSize, -halfSize);
    glVertex3f(halfSize, -halfSize, -halfSize);

    // Top face
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-halfSize, halfSize, -halfSize);
    glVertex3f(-halfSize, halfSize, halfSize);
    glVertex3f(halfSize, halfSize, halfSize);
    glVertex3f(halfSize, halfSize, -halfSize);

    // Bottom face
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-halfSize, -halfSize, -halfSize);
    glVertex3f(halfSize, -halfSize, -halfSize);
    glVertex3f(halfSize, -halfSize, halfSize);
    glVertex3f(-halfSize, -halfSize, halfSize);

    // Right face
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(halfSize, -halfSize, -halfSize);
    glVertex3f(halfSize, halfSize, -halfSize);
    glVertex3f(halfSize, halfSize, halfSize);
    glVertex3f(halfSize, -halfSize, halfSize);

    // Left face
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-halfSize, -halfSize, -halfSize);
    glVertex3f(-halfSize, -halfSize, halfSize);
    glVertex3f(-halfSize, halfSize, halfSize);
    glVertex3f(-halfSize, halfSize, -halfSize);
    glEnd();

    glPopMatrix();
}

void Renderer3D::DrawPyramid(float x, float y, float z, float size, float r, float g, float b) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glColor3f(r, g, b);

    float halfSize = size * 0.5f;
    glBegin(GL_TRIANGLES);

    // Front
    glNormal3f(0.0f, 0.5f, 0.5f);
    glVertex3f(0.0f, halfSize, 0.0f);
    glVertex3f(-halfSize, -halfSize, halfSize);
    glVertex3f(halfSize, -halfSize, halfSize);

    // Right
    glNormal3f(0.5f, 0.5f, 0.0f);
    glVertex3f(0.0f, halfSize, 0.0f);
    glVertex3f(halfSize, -halfSize, halfSize);
    glVertex3f(halfSize, -halfSize, -halfSize);

    // Back
    glNormal3f(0.0f, 0.5f, -0.5f);
    glVertex3f(0.0f, halfSize, 0.0f);
    glVertex3f(halfSize, -halfSize, -halfSize);
    glVertex3f(-halfSize, -halfSize, -halfSize);

    // Left
    glNormal3f(-0.5f, 0.5f, 0.0f);
    glVertex3f(0.0f, halfSize, 0.0f);
    glVertex3f(-halfSize, -halfSize, -halfSize);
    glVertex3f(-halfSize, -halfSize, halfSize);
    glEnd();

    // Bottom
    glBegin(GL_QUADS);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-halfSize, -halfSize, halfSize);
    glVertex3f(halfSize, -halfSize, halfSize);
    glVertex3f(halfSize, -halfSize, -halfSize);
    glVertex3f(-halfSize, -halfSize, -halfSize);
    glEnd();

    glPopMatrix();
}

void Renderer3D::DrawSphere(float x, float y, float z, float radius, float r, float g, float b) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glColor3f(r, g, b);

    GLUquadric* quadric = gluNewQuadric();
    gluQuadricNormals(quadric, GLU_SMOOTH);
    gluSphere(quadric, radius, 32, 32);
    gluDeleteQuadric(quadric);

    glPopMatrix();
}