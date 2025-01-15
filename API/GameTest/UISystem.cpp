//------------------------------------------------------------------------
// UISystem.cpp
//------------------------------------------------------------------------
#include "stdafx.h"
#include "UISystem.h"
#include "Renderer3D.h"
#include <glut/include/GL/glut.h>
#include <App/AppSettings.h>

UIText::UIText(const std::string& txt, float xPos, float yPos, float red, float green, float blue) {
    text = txt;
    x = xPos;
    y = yPos;
    r = red;
    g = green;
    b = blue;
    visible = true;
    font = GLUT_BITMAP_HELVETICA_12;
}

void UIText::Render() {
    if (!visible) return;

    // Save current GL states
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    // Disable depth test and lighting for consistent text rendering
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    // Set blending for text
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // For bold effect, render the text multiple times with slight offsets
    const int BOLD_OFFSET = 2;
    for (int i = 0; i < BOLD_OFFSET; i++) {
        // Set color and position with slight offset for bold effect
        glColor3f(r, g, b);
        glRasterPos2f(x + i, y);

        for (char c : text) {
            glutBitmapCharacter(font, c);
        }
    }

    // Restore previous GL states
    glPopAttrib();
}

UIButton::UIButton(const std::string& txt, float xPos, float yPos, float w, float h, std::function<void()> callback) {
    text = txt;
    x = xPos;
    y = yPos;
    width = w;
    height = h;
    onClick = callback;
    visible = true;
    isHovered = false;
    r = 0.7f;
    g = 0.7f;
    b = 0.7f;
}

void UIButton::Render() {
    if (!visible) return;

    // Draw button background
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();

    // Draw button border
    if (isHovered) {
        glColor3f(1.0f, 1.0f, 1.0f);
    }
    else {
        glColor3f(0.5f, 0.5f, 0.5f);
    }
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();

    // Draw text
    glColor3f(0.0f, 0.0f, 0.0f);
    float textX = x + (width - text.length() * 8) / 2;  // Center text
    float textY = y + (height - 12) / 2;  // Center text
    glRasterPos2f(textX, textY);
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
    }
}

bool UIButton::IsPointInside(float px, float py) {
    return (px >= x && px <= x + width && py >= y && py <= y + height);
}

UISystem::UISystem(Renderer3D* renderer) : renderer3D(renderer) {
}

UISystem::~UISystem() {
    for (auto element : elements) {
        delete element;
    }
    elements.clear();
}

UIText* UISystem::AddText(const std::string& text, float x, float y, float r, float g, float b) {
    UIText* textElement = new UIText(text, x, y, r, g, b);
    elements.push_back(textElement);
    return textElement;
}

UIText* UISystem::AddBoldText(const std::string& text, float x, float y, float r, float g, float b, void* font) {
    UIText* textElement = new UIText(text, x, y, r, g, b);
    textElement->font = font;
    elements.push_back(textElement);
    return textElement;
}

UIButton* UISystem::AddButton(const std::string& text, float x, float y,
    float width, float height, std::function<void()> callback) {
    UIButton* button = new UIButton(text, x, y, width, height, callback);
    elements.push_back(button);
    return button;
}

void UISystem::RemoveElement(UIElement* element) {
    auto it = std::find(elements.begin(), elements.end(), element);
    if (it != elements.end()) {
        delete* it;
        elements.erase(it);
    }
}

void UISystem::SetupOrthoProjection() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    // Always use virtual resolution for UI
    const float FIXED_ASPECT = (float)APP_VIRTUAL_WIDTH / (float)APP_VIRTUAL_HEIGHT;
    float scale = 1.0f;

    // Calculate viewport size and position
    int windowWidth, windowHeight;
    windowWidth = glutGet(GLUT_WINDOW_WIDTH);
    windowHeight = glutGet(GLUT_WINDOW_HEIGHT);
    float currentAspect = (float)windowWidth / (float)windowHeight;

    int viewportX = 0;
    int viewportY = 0;
    int viewportWidth = windowWidth;
    int viewportHeight = windowHeight;

    if (currentAspect > FIXED_ASPECT) {
        viewportWidth = (int)(windowHeight * FIXED_ASPECT);
        viewportX = (windowWidth - viewportWidth) / 2;
        scale = (float)viewportHeight / APP_VIRTUAL_HEIGHT;
    }
    else {
        viewportHeight = (int)(windowWidth / FIXED_ASPECT);
        viewportY = (windowHeight - viewportHeight) / 2;
        scale = (float)viewportWidth / APP_VIRTUAL_WIDTH;
    }

    glViewport(viewportX, viewportY, viewportWidth, viewportHeight);
    glOrtho(0, APP_VIRTUAL_WIDTH, APP_VIRTUAL_HEIGHT, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}

void UISystem::RestorePerspectiveProjection() {
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void UISystem::BeginUI() {
    glDisable(GL_DEPTH_TEST);
    SetupOrthoProjection();
}

void UISystem::EndUI() {
    RestorePerspectiveProjection();
    glEnable(GL_DEPTH_TEST);
}

void UISystem::Render() {
    BeginUI();

    for (auto element : elements) {
        element->Render();
    }

    EndUI();
}

void UISystem::HandleMouseMove(float x, float y) {
    for (auto element : elements) {
        UIButton* button = dynamic_cast<UIButton*>(element);
        if (button) {
            button->isHovered = button->IsPointInside(x, y);
        }
    }
}

void UISystem::HandleMouseClick(float x, float y) 
{
    for (auto element : elements)
    {
        UIButton* button = dynamic_cast<UIButton*>(element);
        if (button->IsPointInside(x, y))
        {
            if (button->onClick)
            {
                button->onClick();
            }
        }
    }
}