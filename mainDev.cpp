#include <Unified-Engine/core/instance.h>

#include <Unified-Engine/Objects/gameObject.h>
#include <Unified-Engine/Objects/Mesh/Defaults/cube.h>
#include <Unified-Engine/Core/Rendering/shader.h>
#include <Unified-Engine/Objects/Components/shaderObject.h>

#include <Unified-Engine/input/input.h>

#include <Unified-Engine/Objects/scriptObject.h>
#include <Unified-Engine/Core/time.h>

class CameraControl : public UnifiedEngine::ScriptableObject{
public:
    UnifiedEngine::GameObject* OBJ;

    CameraControl(UnifiedEngine::GameObject* Par, UnifiedEngine::ObjectComponent* Camera)
        : UnifiedEngine::ScriptableObject(Par)
    {
        OBJ = Par;

        OBJ = Par;
        OBJ->Name = "CamCam";

        UnifiedEngine::instantiate(OBJ);

        this->OBJ->Children.push_back(Camera);
        Camera->Parent = OBJ;
    }

    int Update(){
        //Player Movement
        if (UnifiedEngine::__GAME__GLOBAL__INSTANCE__->Input->Keyboard.KeyPressed(UnifiedEngine::Key_W))
            this->OBJ->transform.Position += 1.f*UnifiedEngine::__GAME__GLOBAL__INSTANCE__->GetMainCamera()->transform.front() * UnifiedEngine::Time.DeltaTime;
        if (UnifiedEngine::__GAME__GLOBAL__INSTANCE__->Input->Keyboard.KeyPressed(UnifiedEngine::Key_A))
            this->OBJ->transform.Position -= 1.f*UnifiedEngine::__GAME__GLOBAL__INSTANCE__->GetMainCamera()->transform.right() * UnifiedEngine::Time.DeltaTime;
        if (UnifiedEngine::__GAME__GLOBAL__INSTANCE__->Input->Keyboard.KeyPressed(UnifiedEngine::Key_S))
            this->OBJ->transform.Position -= 1.f*UnifiedEngine::__GAME__GLOBAL__INSTANCE__->GetMainCamera()->transform.front() * UnifiedEngine::Time.DeltaTime;
        if (UnifiedEngine::__GAME__GLOBAL__INSTANCE__->Input->Keyboard.KeyPressed(UnifiedEngine::Key_D))
            this->OBJ->transform.Position += 1.f*UnifiedEngine::__GAME__GLOBAL__INSTANCE__->GetMainCamera()->transform.right() * UnifiedEngine::Time.DeltaTime;
        if (UnifiedEngine::__GAME__GLOBAL__INSTANCE__->Input->Keyboard.KeyPressed(UnifiedEngine::Key_SPACE))
            this->OBJ->transform.Position += 1.f*UnifiedEngine::__GAME__GLOBAL__INSTANCE__->GetMainCamera()->transform.up() * UnifiedEngine::Time.DeltaTime;
        if (UnifiedEngine::__GAME__GLOBAL__INSTANCE__->Input->Keyboard.KeyPressed(UnifiedEngine::Key_LEFT_SHIFT))
            this->OBJ->transform.Position -= 1.f*UnifiedEngine::__GAME__GLOBAL__INSTANCE__->GetMainCamera()->transform.up() * UnifiedEngine::Time.DeltaTime;
        

        //Player Rotation
        glm::vec2 mPos = UnifiedEngine::__GAME__GLOBAL__INSTANCE__->Input->Mouse.GetPosition();

        //Update pitch yaw and roll
        glm::vec3 threeDRotation = glm::vec3(-static_cast<GLfloat>(mPos.y) * 10.f * UnifiedEngine::Time.DeltaTime, -static_cast<GLfloat>(mPos.x) * 10.f * UnifiedEngine::Time.DeltaTime, 0);
        UnifiedEngine::__GAME__GLOBAL__INSTANCE__->GetMainCamera()->transform.Rotate(threeDRotation);

        //Resolution with scroll
        UnifiedEngine::WindowConfig conf = UnifiedEngine::__GAME__GLOBAL__INSTANCE__->__windows.front()->Config();

        int scroll = UnifiedEngine::__GAME__GLOBAL__INSTANCE__->Input->Mouse.Scroll();

        if(scroll == 1){
            conf.res_x *= 2;
            conf.res_y *= 2;
        }
        else if(scroll == -1){
            conf.res_x /= 2;
            conf.res_y /= 2;
        }

        if (conf.res_x <= 80)
            conf.res_x = 80;
        if (conf.res_y <= 45)
            conf.res_y = 45;

        if (conf.res_x >= 1280)
            conf.res_x = 1280;
        if (conf.res_y >= 720)
            conf.res_y = 720;

        UnifiedEngine::__GAME__GLOBAL__INSTANCE__->__windows.front()->LoadWindowConfig(conf);

        return 0;
    }
};

int main(){
    UnifiedEngine::__GLOBAL_CONFIG__.VersionMajor = 4;
    UnifiedEngine::__GLOBAL_CONFIG__.VersionMinor = 1;

    UnifiedEngine::__INIT__ENGINE();

    // Window Setup
    UnifiedEngine::WindowConfig WinConf = {.x = 1280, .y = 720, .res_x = 0, .res_y = 0, .title = (char*)"Test Program", .resizable = true, .fullScreen = false, .vsync = true, .fps = 60};
    WinConf.backgroundColor = {0, 0, 0};

    UnifiedEngine::Window GameWindow(WinConf);

    // Skybox
    UnifiedEngine::SkyboxSolidColor box(glm::vec3(135, 206, 235));
    UnifiedEngine::__GAME__GLOBAL__INSTANCE__->skybox = &box;

    //Camera
    UnifiedEngine::Camera Cam;
    UnifiedEngine::instantiate(&Cam);

    UnifiedEngine::Shader shader;
    UnifiedEngine::ShaderObject shaderObj(&shader);

    // Create the mesh using the standard cube
    UnifiedEngine::Mesh mesh;
    mesh = UnifiedEngine::Cube();

    // Create a game object that is in view
    UnifiedEngine::GameObject gOBJ(mesh, &shaderObj);
    gOBJ.transform.Position.x -= 5;
    gOBJ.transform.SetRotation(glm::vec3(0, 0, 0));
    UnifiedEngine::instantiate(&gOBJ);

    // Lock the mouse
    UnifiedEngine::InputPointer->Mouse.Cursor.Locked();

    UnifiedEngine::GameObject CamOBJ(UnifiedEngine::Mesh{}, nullptr);
    CameraControl Controller(&CamOBJ, &Cam);

    // Debug window enable
    if(false){
        UnifiedEngine::WindowConfig DbgWinConf = {.x = 1260, .y = 720, .res_x = 0, .res_y = 0, .title = (char*)"Debugger", .resizable = true, .fullScreen = false, .vsync = true, .fps = 60};
        DbgWinConf.backgroundColor = {0, 0, 0};

        UnifiedEngine::Debug::DebugWindow debugWindow(DbgWinConf);
        UnifiedEngine::__GAME__GLOBAL__INSTANCE__->debugWindow = &debugWindow;
    }

    // Game loop
    while (!glfwWindowShouldClose(GameWindow.Context()))
    {
        // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
        if(UnifiedEngine::__GAME__GLOBAL__INSTANCE__->Update()){
            FAULT("GAME_INSTANCE::FAILED TO UPDATE!");
            exit(10);
            //TODO: Call some sort of deinitializer (UnifiedEngine::Terminate())
        }

        if(UnifiedEngine::InputPointer->Keyboard.KeyPressed(UnifiedEngine::Key_ESCAPE)){
            break;
        }

        if(UnifiedEngine::__GAME__GLOBAL__INSTANCE__->Render()){
            FAULT("GAME_INSTANCE::FAILED TO RENDER!");
            //TODO: Call some sort of deinitializer (UnifiedEngine::Terminate())
            exit(11);
        }
        gOBJ.transform.SLerp(Cam.transform.Position, 1 * UnifiedEngine::Time.DeltaTime);
        gOBJ.transform.Move(glm::vec3(5 * UnifiedEngine::Time.DeltaTime, 0, 3 * UnifiedEngine::Time.DeltaTime));
    }

    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();

    return 0;
}
