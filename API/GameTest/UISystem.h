//------------------------------------------------------------------------
// UISystem.h
//------------------------------------------------------------------------
#ifndef UI_SYSTEM_H
#define UI_SYSTEM_H

#include <string>
#include <vector>
#include <functional>
#include <include/GL/freeglut_std.h>

// Forward declarations
class Renderer3D;

struct UIElement {
    float x, y;
    bool visible;
    virtual void Render() = 0;
    virtual ~UIElement() {}
};

struct UIText : public UIElement {
    UIText(const std::string& txt, float xPos, float yPos,
        float red = 1.0f, float green = 1.0f, float blue = 1.0f);
    virtual void Render() override;
    
    std::string text;
    float r, g, b;
    void* font;  // GLUT font
};

struct UIButton : public UIElement {
    std::string text;
    float width, height;
    float r, g, b;
    std::function<void()> onClick;
    bool isHovered;

    UIButton(const std::string& txt, float xPos, float yPos,
        float w, float h, std::function<void()> callback);
    virtual void Render() override;
    bool IsPointInside(float px, float py);
};

class UISystem {
public:
    UISystem(Renderer3D* renderer);
    ~UISystem();

    // UI Element Management
    UIText* AddText(const std::string& text, float x, float y,
        float r = 1.0f, float g = 1.0f, float b = 1.0f);
    UIText* AddBoldText(const std::string& text, float x, float y, float r = 1.0f, float g = 1.0f, float b = 1.0f, void* font = GLUT_BITMAP_HELVETICA_12);
    UIButton* AddButton(const std::string& text, float x, float y,
        float width, float height, std::function<void()> callback);
    void RemoveElement(UIElement* element);

    // Rendering
    void Render();
    void BeginUI();
    void EndUI();

    // Input handling
    void HandleMouseMove(float x, float y);
    void HandleMouseClick(float x, float y);

private:
    std::vector<UIElement*> elements;
    Renderer3D* renderer3D;

    void SetupOrthoProjection();
    void RestorePerspectiveProjection();
};

#endif