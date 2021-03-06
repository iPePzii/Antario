#include "Menu.h"
#include "Settings.h"

#include "SDK\IVEngineClient.h"

// Defined to avoid including windowsx.h
#define GET_X_LPARAM(lp)                        ((float)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)                        ((float)(short)HIWORD(lp))

// Init our statics
std::unique_ptr<MouseCursor> MenuMain::mouseCursor;
MenuStyle MenuMain::style;
CD3DFont* MenuMain::pFont = nullptr;

void Detach() { g_Settings.bCheatActive = false; }


void MouseCursor::Render()
{
    float x = this->vecPointPos.x;
    float y = this->vecPointPos.y;

    // Draw inner fill color
    Vector2D vecPos1 = Vector2D(x + 1, y + 1);
    Vector2D vecPos2 = Vector2D(x + 25, y + 12);
    Vector2D vecPos3 = Vector2D(x + 12, y + 25);
    g_Render.TriangleFilled(vecPos1, vecPos2, vecPos3, g_Settings.colCursor);

    // Draw second smaller inner fill color
    vecPos1 = Vector2D(x + 6, y + 6);
    vecPos2 = Vector2D(x + 19, y + 12);
    vecPos3 = Vector2D(x + 12, y + 19);
    g_Render.TriangleFilled(vecPos1, vecPos2, vecPos3, g_Settings.colCursor);

    // Draw border
    g_Render.Triangle(Vector2D(x, y), Vector2D(x + 25, y + 12), Vector2D(x + 12, y + 25), Color(0, 0, 0, 200));
    //g_Render.Line(x, y, x + 25, y + 12, Color(0, 0, 0, 200));
    //g_Render.Line(x, y, x + 12, y + 25, Color(0, 0, 0, 200));
    //g_Render.Line(x + 25, y + 12, x + 12, y + 25, Color(0, 0, 0, 200));
}



void MouseCursor::RunThink(UINT uMsg, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_MOUSEMOVE:
    case WM_NCMOUSEMOVE:
        this->SetPosition(Vector2D(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
        break;
    case WM_LBUTTONDOWN:
        this->bLMBPressed = true;
        break;
    case WM_LBUTTONUP:
        this->bLMBPressed = false;
        break;
    case WM_RBUTTONDOWN:
        this->bRMBPressed = true;
        break;
    case WM_RBUTTONUP:
        this->bRMBPressed = false;
        break;
    }
}



bool MouseCursor::IsInBounds(Vector2D vecDst1, Vector2D vecDst2)
{
        if (vecPointPos.x > vecDst1.x && vecPointPos.x < vecDst2.x && vecPointPos.y > vecDst1.y && vecPointPos.y < vecDst2.y)
            return true;
        else
            return false;
}



BaseWindow::BaseWindow(Vector2D vecPosition, Vector2D vecSize, CD3DFont* pFont, CD3DFont* pHeaderFont, std::string strLabel)
{
    this->pFont = pFont;
    this->pHeaderFont = pHeaderFont;
    this->strLabel = strLabel;
    this->vecSize = vecSize;
    this->iHeaderHeight = this->GetHeaderHeight();
    this->SetPos(vecPosition);
}



void BaseWindow::Render()
{
    // Draw main background rectangle
    g_Render.RectFilledGradient(this->vecPosition, this->vecPosition + this->vecSize, Color(50, 50, 50, 255), Color(20, 20, 20, 255), GradientType::GRADIENT_VERTICAL);
    // Draw header rect.
    g_Render.RectFilledGradient(this->vecPosition, Vector2D(this->vecPosition.x + this->vecSize.x, this->vecPosition.y + this->iHeaderHeight), Color(50, 50, 50, 250), Color(20, 20, 20, 250), GradientType::GRADIENT_VERTICAL);

    // Draw header string, defined as label.
    g_Render.String(this->vecPosition.x +(this->vecSize.x * 0.5f), this->vecPosition.y, CD3DFONT_CENTERED_X, this->style.colHeaderText, this->pHeaderFont, this->strLabel.c_str());
    
    // Render all childrens
    MenuMain::Render();
}



void BaseWindow::UpdateData()
{
    static bool bIsInitialized = false;
    Vector2D vecHeaderBounds = Vector2D(this->vecPosition.x + this->vecSize.x, this->vecPosition.y + this->iHeaderHeight);


    auto SetChildPos = [&]()    // Set the position of all child buttons / checkboxes etc.
    {
        for (std::size_t it = 0; it < this->vecChildren.size(); it++)
        {
            this->vecChildren.at(it)->SetPos(Vector2D(this->vecPosition.x + 10,
                this->vecPosition.y + (it * this->vecChildren.at(it)->GetSize().y) + this->iHeaderHeight + 10));
        }
    };
    if (!bIsInitialized)
        SetChildPos();


    // Check if mouse has been pressed in the proper area. If yes, save window state as dragged.
    if (this->mouseCursor->bLMBPressed && MenuMain::mouseCursor->IsInBounds(this->vecPosition, vecHeaderBounds))
        this->bIsDragged = true;
    else
    if (!this->mouseCursor->bLMBPressed)
        this->bIsDragged = false;


    // Check if the window is dragged. If it is, move window by the cursor difference between ticks.
    static Vector2D vecOldMousePos = this->mouseCursor->vecPointPos;
    if (this->bIsDragged)
    {
        Vector2D vecNewPos = this->vecPosition + (this->mouseCursor->vecPointPos - vecOldMousePos);
        this->SetPos(vecNewPos);
        SetChildPos();
        vecOldMousePos = this->mouseCursor->vecPointPos;
    }
    else
        vecOldMousePos = this->mouseCursor->vecPointPos;
    
    // Call the inherited "UpdateData" function from the MenuMain class to loop through childs
    MenuMain::UpdateData();
}



int BaseWindow::GetHeaderHeight()
{
    SIZE size;
    this->pHeaderFont->GetTextExtent(this->strLabel.c_str(), &size);
    return size.cy + 1;
}



void MenuMain::SetParent(MenuMain* newParent)
{
    this->pParent = newParent;
}



void MenuMain::AddChild(std::shared_ptr<MenuMain> child)
{
    child->SetParent(this);
    this->vecChildren.push_back(child);
}



void MenuMain::Render()
{  
    // Render all childrens
    for (std::size_t it = 0; it < this->vecChildren.size(); it++)
        this->vecChildren.at(it)->Render();
}



void MenuMain::RunThink(UINT uMsg, LPARAM lParam)
{
    this->mouseCursor->RunThink(uMsg, lParam);
/// TODO: Capture keyboard input
}



void MenuMain::Initialize()
{
    std::shared_ptr<BaseWindow> mainWindow = std::make_shared<BaseWindow>(Vector2D(450, 450), Vector2D(360, 256), g_Fonts.pFontTahoma8.get(), g_Fonts.pFontTahoma10.get(), "Antario - Main"); // Create our main window (Could have multiple if you'd create vec. for it)
    {
        mainWindow->AddChild(std::make_unique<Checkbox> ("Bunnyhop", &g_Settings.bBhopEnabled));
        mainWindow->AddChild(std::make_unique<Button>   ("Shutdown", Detach));
        // All child menus / buttons etc, will be done in the future.
    }
    this->AddChild(mainWindow);

    this->mouseCursor = std::make_unique<MouseCursor>();    // Create our mouse cursor (one instance only)
}



void MenuMain::UpdateData()
{
    if (this->vecChildren.size() > 0)
    {
        for (auto it = this->vecChildren.begin(); it != this->vecChildren.end(); it++)
            (*it)->UpdateData();
    }
}



Checkbox::Checkbox(std::string strLabel, bool *bValue)
{
    this->strLabel = strLabel;
    this->bCheckboxValue = bValue;

    SIZE size;
    this->pFont->GetTextExtent(this->strLabel.c_str(), &size);
    this->vecSize = Vector2D(100, static_cast<float>(size.cy) + this->style.iPaddingY);
    this->vecSelectableSize = Vector2D(static_cast<float>(size.cy), static_cast<float>(size.cy));
}



void Checkbox::Render()
{
    if (*this->bCheckboxValue)
        g_Render.RectFilledGradient(this->vecPosition + 1, this->vecPosition + this->vecSelectableSize, this->style.colCheckbox1, this->style.colCheckbox2, GradientType::GRADIENT_VERTICAL);

    g_Render.Rect(this->vecPosition, this->vecPosition + this->vecSelectableSize, Color(15, 15, 15, 220));    
    g_Render.String((this->vecPosition.x + this->vecSelectableSize.x + 5), this->vecPosition.y, CD3DFONT_DROPSHADOW, this->style.colText, this->pFont, this->strLabel.c_str());

    if (this->bIsHovered)
        g_Render.RectFilled(this->vecPosition + 1, this->vecPosition + this->vecSelectableSize, Color(100, 100, 100, 50));
}



void Checkbox::UpdateData()
{
    static bool bIsChanged = false;

    if (MenuMain::mouseCursor->IsInBounds(this->vecPosition, (this->vecPosition + this->vecSize)))
    {
        if (this->mouseCursor->bLMBPressed && !bIsChanged)
        {
            *this->bCheckboxValue = !*this->bCheckboxValue;
            bIsChanged = true;
        }
        else
        if (!this->mouseCursor->bLMBPressed && bIsChanged)
            bIsChanged = false;

        this->bIsHovered = true;
    }
    else
        this->bIsHovered = false;
}



Button::Button(std::string strLabel, void (&fnPointer)())
{
    this->strLabel = strLabel;
    this->fnActionPlay = fnPointer;

    SIZE size;
    this->pFont->GetTextExtent(this->strLabel.c_str(), &size);
    this->vecSize = Vector2D(100, static_cast<float>(size.cy) + this->style.iPaddingY);
}



void Button::Render()
{
    g_Render.RectFilledGradient(this->vecPosition, this->vecPosition + this->vecSize, this->style.colCheckbox1, this->style.colCheckbox2, GradientType::GRADIENT_VERTICAL);
    g_Render.Rect(this->vecPosition, this->vecPosition + this->vecSize, Color(15, 15, 15, 220));
    g_Render.String(this->vecPosition.x + this->vecSize.x / 2.f, this->vecPosition.y + this->vecSize.y / 2.f, CD3DFONT_DROPSHADOW | CD3DFONT_CENTERED_X | CD3DFONT_CENTERED_Y, this->style.colText, this->pFont, this->strLabel.c_str());

    if (this->bIsHovered)
        g_Render.RectFilled(this->vecPosition + 1, this->vecPosition + this->vecSize, Color(100, 100, 100, 50));
}



void Button::UpdateData()
{
    static bool bIsActivated = false;

    if (MenuMain::mouseCursor->IsInBounds(this->vecPosition, (this->vecPosition + this->vecSize)))
    {
        if (this->mouseCursor->bLMBPressed && !bIsActivated)
        {
            this->fnActionPlay();   // Run the function passed as an arg.
            bIsActivated = true;
        }
        else
        if (!this->mouseCursor->bLMBPressed && bIsActivated)
            bIsActivated = false;

        this->bIsHovered = true;
    }
    else
        this->bIsHovered = false;
}
