#include "stdafx.h"
#include <windows.h> 
#include <math.h>  
#include "app/app.h"
#include "Renderer3D.h"
#include "UISystem.h"
#include "Galaxy.h"
#include "Spaceship.h"

// Global variables
Renderer3D* renderer = nullptr;
UISystem* ui = nullptr;
Galaxy* galaxy = nullptr;
Spaceship* spaceship = nullptr;

UIText* fpsDisplay = nullptr;
UIText* positionDisplay = nullptr;
UIText* mousePositionDisplay = nullptr;
UIText* healthDisplay = nullptr;
UIText* ammoDisplay = nullptr;
UIText* gameOverText = nullptr;
UIText* restartText = nullptr;

float mouseWorldX = 0.0f;
float mouseWorldY = 0.0f;

// Constants
const float MOVE_SPEED = 2.0f;
const float CAM_HEIGHT = 0.0f;
const float CAM_DISTANCE = 150.0f;
const float INITIAL_ZOOM = 1500.0f;  // Starting zoom distance
const float ZOOM_DURATION = 2.0f;   // Time in seconds for zoom effect
float currentZoomTime = 0.0f;       // Track zoom animation progress
bool isZooming = true;             // Track if initial zoom is active

//------------------------------------------------------------------------
// Called before first update. Do any initial setup here.
//------------------------------------------------------------------------
void Init() {
    // Initialize basic systems
    renderer = new Renderer3D();
    renderer->Initialize(APP_VIRTUAL_WIDTH, APP_VIRTUAL_HEIGHT);

    // Initialize UI and add text displays
    ui = new UISystem(renderer);
    fpsDisplay = ui->AddText("FPS: 0", 10, 20);

    const float TEXT_PADDING_X = 50.0f;  // Distance from left edge
    const float TEXT_Y_POS = APP_VIRTUAL_HEIGHT / 2;  // Vertical center

    gameOverText = ui->AddBoldText("GAME OVER",
        TEXT_PADDING_X,
        TEXT_Y_POS - 50,  // Slightly above center
        1.0f, 0.0f, 0.0f,  // Red color
        GLUT_BITMAP_TIMES_ROMAN_24);

    restartText = ui->AddText("Press R to Restart",
        TEXT_PADDING_X,
        TEXT_Y_POS + 20,  // Slightly below center
        1.0f, 1.0f, 1.0f);

    gameOverText->visible = false;
    restartText->visible = false;

    mousePositionDisplay = ui->AddText("Mouse: 0, 0", APP_VIRTUAL_WIDTH/2 - 100, APP_VIRTUAL_HEIGHT - 10);

    // Create galaxy and spaceship
    galaxy = new Galaxy(renderer, 100);
    spaceship = new Spaceship();

    healthDisplay = ui->AddText("Ship Health: " + spaceship->health , 10, APP_VIRTUAL_HEIGHT - 30);
    positionDisplay = ui->AddText("Ship Position: 0, 0, 0", 10, APP_VIRTUAL_HEIGHT - 50);
    ammoDisplay = ui->AddText("Ship Ammo: " + std::to_string(spaceship->MAX_AMMO), 10, APP_VIRTUAL_HEIGHT - 10);

    // Set initial positions
    spaceship->SetPosition(0.0f, 0.0f, 0.0f);

    // Set initial camera position
    Camera& camera = renderer->GetCamera();
    camera.SetPosition(0.0f, 0.0f, INITIAL_ZOOM);
    camera.SetRotation(0.0f, 0.0f, 0.0f);
}

//------------------------------------------------------------------------
// Update camera position to follow spaceship
//------------------------------------------------------------------------
void UpdateCamera(float deltaTime) {
    Camera& camera = renderer->GetCamera();
    float camX, camY, camZ;
    float shipX, shipY, shipZ;

    camera.GetPosition(camX, camY, camZ);
    spaceship->GetPosition(shipX, shipY, shipZ);

    // Handle initial zoom animation
    if (isZooming) {
        currentZoomTime += deltaTime * 0.001f;;
        if (currentZoomTime >= ZOOM_DURATION) {
            isZooming = false;
            currentZoomTime = ZOOM_DURATION;
        }

        // Calculate zoom progress (0 to 1) with easing
        float progress = currentZoomTime / ZOOM_DURATION;
        progress = 1.0f - (1.0f - progress) * (1.0f - progress); // Ease out quadratic

        // Interpolate between initial zoom and final camera distance
        float currentDistance = INITIAL_ZOOM + (CAM_DISTANCE - INITIAL_ZOOM) * progress;

        // Calculate target camera position with current zoom level
        float targetX = shipX;
        float targetY = shipY + CAM_HEIGHT;
        float targetZ = shipZ + currentDistance;

        // Instant camera movement during zoom
        camX = targetX;
        camY = targetY;
        camZ = targetZ;

        camera.SetPosition(camX, camY, camZ);
    }
    else {
        // Normal gameplay camera following
        float targetX = shipX;
        float targetY = shipY + CAM_HEIGHT;
        float targetZ = shipZ + CAM_DISTANCE;

        // Smooth camera movement
        float lag = 0.05f;
        camX = camX * (1 - lag) + targetX * lag;
        camY = camY * (1 - lag) + targetY * lag;
        camZ = camZ * (1 - lag) + targetZ * lag;

        camera.SetPosition(camX, camY, camZ);
    }

    // Calculate angle to look at ship
    float dx = shipX - camX;
    float dy = shipY - camY;
    float dz = shipZ - camZ;
    float distance = sqrt(dx * dx + dy * dy + dz * dz);

    if (distance > 0) {
        float pitch = atan2f(dy, sqrt(dx * dx + dz * dz)) * (180.0f / 3.14159f);
        float yaw = -atan2f(dx, sqrt(dx * dx + dz * dz)) * 180.0f / 3.14159f;
        camera.SetRotation(pitch, yaw, 0.0f);
    }
}

void DrawCrosshair(float mouseX, float mouseY) {

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, APP_VIRTUAL_WIDTH, APP_VIRTUAL_HEIGHT, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    mouseY = APP_VIRTUAL_HEIGHT - mouseY;

    glColor3f(1.0f, 1.0f, 1.0f);  // Color
    glLineWidth(Spaceship::CROSSHAIR_THICKNESS);

    // Draw circle
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < Spaceship::CIRCLE_SEGMENTS; i++) {
        float angle = i * 2.0f * 3.14159f / Spaceship::CIRCLE_SEGMENTS;
        float x = mouseX + Spaceship::CROSSHAIR_RADIUS * cosf(angle);
        float y = mouseY + Spaceship::CROSSHAIR_RADIUS * sinf(angle);
        glVertex2f(x, y);
    }
    glEnd();

    // Draw plus sign
    glBegin(GL_LINES);
    // Horizontal line
    glVertex2f(mouseX - Spaceship::CROSSHAIR_LINE_LENGTH, mouseY);
    glVertex2f(mouseX + Spaceship::CROSSHAIR_LINE_LENGTH, mouseY);
    // Vertical line
    glVertex2f(mouseX, mouseY - Spaceship::CROSSHAIR_LINE_LENGTH);
    glVertex2f(mouseX, mouseY + Spaceship::CROSSHAIR_LINE_LENGTH);
    glEnd();

    // Reset line width
    glLineWidth(1.0f);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

//------------------------------------------------------------------------
// Update game state. deltaTime is the elapsed time since the last update in ms.
//------------------------------------------------------------------------
void Update(float deltaTime) {
    // Check for game over conditions
    bool isGameOver = !spaceship->IsAlive() || spaceship->GetAmmo() <= 0;
    float dt = deltaTime * 0.001f;  // Convert to seconds

    // Handle spaceship movement
    float moveX = 0.0f, moveY = 0.0f;

    if (App::IsKeyPressed('W')) moveY = 1.0f;
    if (App::IsKeyPressed('S')) moveY = -1.0f;
    if (App::IsKeyPressed('A')) moveX = -1.0f;
    if (App::IsKeyPressed('D')) moveX = 1.0f;

    // Apply movement if any input
    if (moveX != 0.0f || moveY != 0.0f) {
        spaceship->Move(moveX, moveY);
    }

    float mouseX, mouseY;
    App::GetMousePos(mouseX, mouseY);

    char mouseText[64];
    sprintf_s(mouseText, "Crosshair Pos: %.1f, %.1f", mouseX, mouseY);
    mousePositionDisplay->text = mouseText;

    // Get ship's current data
    float shipX, shipY, shipZ;
    spaceship->GetPosition(shipX, shipY, shipZ);

    char healthText[32];
    sprintf_s(healthText, "Ship Health: %d", spaceship->GetHealth());
    healthDisplay->text = healthText;

    char posText[64];
    sprintf_s(posText, "Ship Position: %.1f, %.1f, %.1f", shipX, shipY, shipZ);
    positionDisplay->text = posText;

    char ammoText[32];
    sprintf_s(ammoText, "Ammo: %d/%d", spaceship->GetAmmo(), spaceship->MAX_AMMO);
    ammoDisplay->text = ammoText;

    // Update components
    spaceship->Update(dt);
    galaxy->Update(deltaTime, renderer->GetCamera());
    UpdateCamera(deltaTime);

    spaceship->LookAt(mouseX, mouseY);

    // Update UI
    static float fps = 0;
    fps = 0.9f * fps + 0.1f * (1000.0f / deltaTime);
    char fpsText[32];
    sprintf_s(fpsText, "FPS: %.1f", fps);
    fpsDisplay->text = fpsText;

    if (gameOverText) gameOverText->visible = isGameOver;
    if (restartText) restartText->visible = isGameOver;
    if (ammoDisplay) ammoDisplay->visible = !isGameOver;
    if (healthDisplay) healthDisplay->visible = !isGameOver;
    //temp off
    if (positionDisplay) positionDisplay->visible = false;
    if (mousePositionDisplay) mousePositionDisplay->visible = false;

    // Handle restart
    if (isGameOver && App::IsKeyPressed('R')) {
        // Reset game state
        delete spaceship;
        spaceship = new Spaceship();
        spaceship->SetPosition(0.0f, 0.0f, 0.0f);

        // Reset zoom animation
        currentZoomTime = 0.0f;
        isZooming = true;

        // Reset UI visibility
        gameOverText->visible = false;
        restartText->visible = false;
    }
}

//------------------------------------------------------------------------
// Render the game world
//------------------------------------------------------------------------
void Render() {
    // Clear screen with dark background
    glClearColor(0.0f, 0.0f, 0.02f, 1.0f);
    renderer->SetupScene();

    // Render game objects
    galaxy->Render();
    spaceship->Render();

    galaxy->SetSpaceship(spaceship);

    // Draw crosshair
    float mouseX, mouseY;
    App::GetMousePos(mouseX, mouseY);
    DrawCrosshair(mouseX, mouseY);

    // Render UI on top
    ui->Render();
}

//------------------------------------------------------------------------
// Clean up resources
//------------------------------------------------------------------------
void Shutdown() {
    delete spaceship;
    delete galaxy;
    delete ui;
    delete renderer;
}