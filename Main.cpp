//directx9 application for C++ using Imgui. This is the main file that starts our window. It's a lot of boilerplate but I really, really enjoyed learning
//about how this library works. It's incredible the amount of heavy lifting that goes into making an image appear on the screen when using C++
//I have a new, but still all too familiarly begrudging, appreciation of Epic Games. They have managed to make C++ be a walk in the park for the 
//average dullard technical artist like I. 

#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include <d3d9.h>
#include <tchar.h>
#include <vector>
#include <sstream> // std::stringstream

using namespace std;
// Data
static LPDIRECT3D9              g_pD3D = nullptr;
static LPDIRECT3DDEVICE9        g_pd3dDevice = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam); //function signature for the Window Procedure, what a rabbit hole. 






int ConfigWindow(int& numResistors) //building out my method to actually instantiate the number of resistors I want and the user would like. 
{
    ImGui::Begin("Configuration");
    ImGui::Text("Select the number of resistors:");
    ImGui::SliderInt("##NumResistors", &numResistors, 1, 10); // Allowing 1 to 10 resistors for example
    if (ImGui::Button("Confirm"))
    {
        ImGui::End();
        return numResistors;
    }
    ImGui::End();
    return -1; // Return -1 to indicate the user hasn't confirmed yet
}



class Resistor //class declaration for the resistors we're going to instantiate. It'd be cool if we did voltage and amperage... I did that initially with 
    //the node library but I realized there was too much to fix with multiple node connections and n* numbers of nodes... The resistance calculation with that
    //would have been insane and probably a senior or junior level course in university. We'll get there!
{
public:
    float resistance;
    bool isParallel;

    Resistor() : resistance(1.0f), isParallel(false) {}
};



void ConfigureResistors(vector<Resistor>& resistors) // configuring the actual resistors in the program. 
{
    for (size_t i = 0; i < resistors.size(); ++i) {
        ImGui::Begin(("Resistor " + to_string(i + 1)).c_str());

        // Slider for resistance value and return to string. 
        ImGui::SliderFloat(("Resistance##" + to_string(i)).c_str(), &resistors[i].resistance, 1.0f, 100.0f);

        // Checkbox for parallel/series and return to string
        ImGui::Checkbox(("Parallel to Previous##" + to_string(i)).c_str(), &resistors[i].isParallel);

        ImGui::End();
    }
}



double calculateTotalResistance(const vector<Resistor>& resistors) // to calculate resistance along the resistor chain. 
{
    double totalResistance = 0;
    for (const auto& resistor : resistors) 
    {
        if (resistor.isParallel) {
            // Resistors in parallel
            totalResistance += 1 / resistor.resistance;
        }
        else {
            // Resistors in series
            totalResistance += resistor.resistance;
        }
    }

    // If resistors are in parallel, take the reciprocal of the total resistance
    if (resistors.front().isParallel) 
    {
        totalResistance = 1 / totalResistance;
    }

    return totalResistance;
}



















// Main code
int main(int, char**)
{
    // Default to 1 resistor for the sake of it. Lots of boilerplate variable declaration. 
    int numResistors = 1; 
    bool configurationConfirmed = false;
    static bool configureResistorsClicked = false;
    vector<Resistor> resistors;
    static bool calculateButtonClicked = false;
    static double totalResistance = 0.0;

    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Woodward Impedance Calculator", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Woodward Impedance Calculator", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance); //scope resolution operator for maximum effect I guess? They're searching for something globally
        //beyond this class or namespace. I guess this is probably best practice to super make sure you're killing the program so it doesn't linger and 
        //start to clog up people's computers. I just saw it in the other deployments. I guess in the case of failure you want to undoubtedly kill the process.
        return 1; 
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION(); // I didn't dive too deep into what this meant exactly, I know that at work, a version check isn't always a brick wall to keep
    //the user from running the program, sometimes you will have error handling for stuff you know will break with new updates coming out and will kind of 
    //be prepared for certain cases on version check. It looks like they're just passing in dimension info from first glance. 
    ImGui::CreateContext(); // the define here is pretty solid, this library really rocks. They made it really easy to learn and use. 
    ImGuiIO& io = ImGui::GetIO(); (void)io; //just guessing this is a housecleaning thing. It's like setting null on stuff after using it in C# for Unity
    //since engine will occasionally freak out. UE4 does the same with certain cases, if you don't explicitly null out certain objects before changing scenes
    //it'll just forget and act like it's supposed to be there when the next scene loads or during a build it's super annoying. 
 
    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f); // cool color config that Imgui has. You can be wild with it too, you can even animate it. 

    // Main loop
    bool done = false;
    while (!done)
    {
       

        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            g_d3dpp.BackBufferWidth = g_ResizeWidth;
            g_d3dpp.BackBufferHeight = g_ResizeHeight;
            g_ResizeWidth = g_ResizeHeight = 0;
            ResetDevice();

            //In a Windows application, the WM_SIZE message is sent to the window procedure when the size of the window is changed
            //While you can handle window resizing directly inside the WM_SIZE handler, there are cases where it might not be ideal to perform resource-intensive operations,
            //such as resizing the back buffer for Direct3D rendering, directly within that message handler. Anything regarding GPU is kind of fickle in my experience and is
            //not always the most reliable act. There is some cool stuff you can do with 3js or if I chose to use openGL a lot of the math being done here could have been 
            //relegated to the graphics card as a shader op. 
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        //my own logic for handling the window popups. Just waits until you've got your info put in and then prompts... Nothing special. 
        if (!configurationConfirmed) 
        {
            configurationConfirmed = (ConfigWindow(numResistors) != -1);
        }
        else 
        {
            if (numResistors > 0) 
            {
                resistors.resize(numResistors); // Resize the vector to hold the specified number of resistors
                ConfigureResistors(resistors);  // Call the correct method to configure resistors
                if (configurationConfirmed && numResistors > 0) 
                {
                    if (ImGui::Button(calculateButtonClicked ? ("Total Resistance: " + to_string(totalResistance) + " ohms").c_str() : "Test resistance among the entire circuit")) 
                    {
                        // Calculate total resistance of the circuit
                        totalResistance = calculateTotalResistance(resistors);
                        calculateButtonClicked = true;
                    }
                    if (ImGui::Button("Configure Resistors")) 
                    {
                        // Reset variables and flags to allow reconfiguration. Wow! A program that resets itself! I've really gone far beyond... (kidding.) 
                        numResistors = 0;
                        resistors.clear();
                        configurationConfirmed = false;
                        calculateButtonClicked = false;
                    }
                }
            }
        }
    

        // Rendering
        //the weird thing about imgui is that it doesn't seem like it was originally written with Windows or DirectX in mind. There are some renormalizations
        //you have to write for the colors as it works on a 0,1 scale versus the 0,255 scale of DX. No complaints or anything but it was an interesting thing
        //to come across. Not sure why they chose a 0,1 in all honesty. 
        ImGui::EndFrame();
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * clear_color.w * 255.0f), (int)(clear_color.y * clear_color.w * 255.0f), (int)(clear_color.z * clear_color.w * 255.0f), (int)(clear_color.w * 255.0f));
        g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }
        HRESULT result = g_pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr);

        // Handle loss of D3D9 device
        if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
            ResetDevice();
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions, I didn't write these, I just swiped them straight from the example project and documentation because I didn't really know what
// good cleanup or config looked like for this. This is my first time using a GUI library that isn't a game engine. This was an impressive struggle LOL. 
//I'll probably have my most verbose comments here as I learned the most here in particular. I didn't get to go as deeply as I would have liked into 
//the more involved graphics library handling that Imgui has employed. Maybe one day! 

bool CreateDeviceD3D(HWND hWnd)
{
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
        return false;

    // Create the D3DDevice
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return false;

    return true;
}

void CleanupDeviceD3D()
{
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = nullptr; }
}

void ResetDevice()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
// Win32 message handler. Did a bit of reading aobut this last night as it was my first time actually coming across it as something I could not afford
//to ignore. This is usually out of my wheelhouse as a tool developer but let's go over what I've gathered about what each of these things are. 
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) 
//This is the window procedure function. It takes four parameters: hWnd: a Handle to the window that recieved the message
//msg, the actual message code-- unsigned int, fancy. wParam and lParam are message specific information. wParam stands for word parameter and it is 
//a 16 bit parameter that carries information about the message. It's sometimes used to handle keycodes of pressed keys, sometimes for mouse messages. 
//lParam is a 32 bit parameter for passing bigger stuff. A mouse click is one thing, but a cartesian position is another. This is used for passing the
//u,v of the mouse on the screen. It also appears to be used for key combinations?  
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE: //wow, take a look at the defines for the msg types-- actual hexadecimal, real fancy. Closest I've ever been to the metal so far! 
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND: //it appears they all refer back to a hexadecimal value. Cool stuff. 
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
