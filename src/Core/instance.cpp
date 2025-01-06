#include <Unified-Engine/Core/instance.h>
#include <Unified-Engine/debug.h>
#include <Unified-Engine/Objects/Components/texture2d.h>
#include <Unified-Engine/Core/time.h>
#include <Unified-Engine/Objects/gameObject.h>

#include <chrono>
#include <thread>

using namespace UnifiedEngine;

namespace UnifiedEngine
{
    GameInstance* __GAME__GLOBAL__INSTANCE__ = nullptr;

    /**
     * @brief Creates a main game engine instance
     * 
     * @return int (-1 for error)
     */
    int __INIT__ENGINE(){
        if(__GAME__GLOBAL__INSTANCE__){
            WARN("ATTEMPTED TO RE-INITIALISE");
            return -1;
        }

        __GAME__GLOBAL__INSTANCE__ = new GameInstance();

        return 0;
    }

    //Enable
	void OpenGLEnable(GLenum ToEnable) {
		glEnable(ToEnable);
	}

	//Disable
	void OpenGLDisable(GLenum ToDisable) {
		glEnable(ToDisable);
	}

	//PolygonMode
	void OpenGLPolygonMode(GLenum Face, GLenum Mode) {
		glPolygonMode(Face, Mode);
	}

	//Blending
	void OpenGLBlend(GLenum SFactor, GLenum DFactor) {
		glBlendFunc(SFactor, DFactor);
	}

	//GLRendering
	void OpenGLRendering(GLenum Cull, GLenum Front) {
		glCullFace(Cull);
		glFrontFace(Front);
	}

    GameInstance::GameInstance(){
        // Init GLFW
        glfwInit();

        // Set all the required options for GLFW
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        #ifdef __APPLE__
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
            glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
        #endif
    }
    GameInstance::~GameInstance(){
        if(this->Framebuffer)
            glDeleteFramebuffers(1, &this->Framebuffer);
        if(this->Renderbuffer)
            glDeleteRenderbuffers(1, &this->Renderbuffer);
        if(this->Texture)
            glDeleteTextures(1, &this->Texture);
    }

    int GameInstance::_Init_Glad(){
        //Load and check if Glad is loaded
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            FAULT("Failed to initialize GLAD");
            return -1;
        }

        gladInited = true;

        //Configure
        OpenGLEnable(GL_DEPTH_TEST);
        OpenGLEnable(GL_CULL_FACE);
        OpenGLRendering(GL_BACK, GL_CCW);
        OpenGLEnable(GL_BLEND);
        OpenGLBlend(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        OpenGLPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        //Init Atlas
        __GLOBAL_ATLAS = new TextureAtlas();

        return 0;
    }

    /**
     * @brief Update and call component updates
     * 
     * @return int 
     */
    int GameInstance::Update(){
        if(!this->__windows.size()){
            FAULT("No Window Object Located");
            return -1;
        }

        if(!this->GetMainCamera()){
            FAULT("No Camera Object Located");
            return -1;
        }

        //Update Time Class
        Time.Update();

        // UpdateDebug Window info
        // Done before content as "DeltaTime" control is possible
        if(this->debugWindow){
            this->debugWindow->Update();
        }

        // Window Context
        this->__windows.front()->Activate();

        //First Update Input
        glfwPollEvents();

        for (auto i = this->objects.begin(); i != this->objects.end(); i++) {
            (*i)->Update();
        }

        //Camera
        this->GetMainCamera()->Update();
        this->ProjectionMatrix = glm::mat4(1.f);
        this->ProjectionMatrix = glm::perspective(glm::radians(this->GetMainCamera()->FOV), static_cast<float>(this->__windows.front()->Config().res_x) / this->__windows.front()->Config().res_y, this->GetMainCamera()->NearPlane, this->GetMainCamera()->FarPlane);

        //Skybox
        if(this->skybox)
            this->skybox->Update();

        return 0;
    }

    /**
     * @brief Draws All Objects to the screen
     * 
     * @return int 
     */
    int GameInstance::Render(){
        if(!this->__windows.size()){
            FAULT("No Window Object Located");
            return -1;
        }

        if(!this->GetMainCamera()){
            FAULT("No Camera Object Located");
            return -1;
        }

        // Window Context
        this->__windows.front()->Activate();

        // Ensure V-Sync is set properly
        if (this->__windows.front()->Config().vsync) {
            glfwSwapInterval(1); // Enable V-Sync
        } else {
            glfwSwapInterval(0); // Disable V-Sync
        }

        if(this->Framebuffer)
            glDeleteFramebuffers(1, &this->Framebuffer);
        if(this->Renderbuffer)
            glDeleteRenderbuffers(1, &this->Renderbuffer);
        if(this->Texture)
            glDeleteTextures(1, &this->Texture);

        // See if we need to scale
        bool Scaled = false;
        if (this->__windows.front()->Config().res_x != this->__windows.front()->Config().x || this->__windows.front()->Config().res_y != this->__windows.front()->Config().y) {
            Scaled = true;
        }

        // Ensure consistent viewport
        glViewport(0, 0, this->__windows.front()->Config().res_x,  this->__windows.front()->Config().res_y);

        //
        // Resolution Stuff
        //
        if (Scaled){
            glGenFramebuffers(1, &this->Framebuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, this->Framebuffer);
        }

        OpenGLEnable(GL_DEPTH_TEST);
        OpenGLEnable(GL_CULL_FACE);

        //
        // Resolution Stuff
        //
        if (Scaled){
            glGenTextures(1, &this->Texture);
            glBindTexture(GL_TEXTURE_2D, this->Texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->__windows.front()->Config().res_x, this->__windows.front()->Config().res_y, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            
            glGenRenderbuffers(1, &this->Renderbuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, this->Renderbuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, this->__windows.front()->Config().res_x, this->__windows.front()->Config().res_y);
            
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->Texture, 0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->Renderbuffer);
        }

        // Render
        if(!this->skybox){
            glClearColor(this->__windows.front()->Config().backgroundColor.red, this->__windows.front()->Config().backgroundColor.green, this->__windows.front()->Config().backgroundColor.blue, this->__windows.front()->Config().backgroundColor.alpha);
            glClear(GL_COLOR_BUFFER_BIT);
            glClear(GL_DEPTH_BUFFER_BIT);
            glClear(GL_STENCIL_BUFFER_BIT);
        }
        else{
            this->skybox->Render();
        }

        // Draw Objects
        for (auto i = this->objects.begin(); i != this->objects.end(); i++) {
            (*i)->Render();
        }

        //
        // Resolution Stuff
        //
        if (Scaled){
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

            OpenGLDisable(GL_DEPTH_TEST);
            OpenGLDisable(GL_CULL_FACE);

            glViewport(0, 0, this->__windows.front()->Config().x,  this->__windows.front()->Config().y);

            glClearColor(0, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT);
            glClear(GL_DEPTH_BUFFER_BIT); // If issues, disable
            // glClear(GL_STENCIL_BUFFER_BIT);

            glBlitFramebuffer(0, 0, this->__windows.front()->Config().res_x, this->__windows.front()->Config().res_y, 0, 0, this->__windows.front()->Config().x, this->__windows.front()->Config().y, GL_COLOR_BUFFER_BIT, GL_NEAREST);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
        }

        // Swap the screen buffers
        glfwSwapBuffers(this->__windows.front()->Context());

        // Render Debugger
        if(this->debugWindow){
            this->debugWindow->Render();
        }

        // Increment Counter
        FrameCounter++;

        // Delay next frame (simplified)
        if (!this->__windows.front()->Config().vsync && this->__windows.front()->Config().fps > 0 && this->LastTime > 0.f) {
            float targetTime = this->LastTime + 1.0f / this->__windows.front()->Config().fps;
            while (Time.Time() < targetTime) {
                std::this_thread::sleep_for(std::chrono::microseconds(100)); // Sleep briefly to avoid overloading the CPU
            }
        }

        // Update for FPS
        this->LastTime = Time.Time();

        // Clean
        glFlush();
        
        return 0;
    }

    
    int GameInstance::RecuseSearchChild(int Setting, bool multi, void* resultstore, std::list<ObjectComponent*>* StartingPoint, void* argument){
        if(!resultstore){
            FAULT("No Result Location Provided");
            return -1;
        }

        switch (Setting)
        {
        case 0: // type
            {    
                if(!argument){
                    FAULT("Missing Comparitor Argument");
                    return -1;
                }

                if(multi){
                    std::list<ObjectComponent*>* resultLoc = (std::list<ObjectComponent*>*)resultstore;

                    for(auto i = StartingPoint->begin(); i != StartingPoint->end(); i++){
                        if((*i)->type == *((ObjectComponentType*)argument)){
                            resultLoc->push_back(*i);
                        }

                        if((*i)->Children.size() > 0){
                            RecuseSearchChild(Setting, multi, resultstore, &((*i)->Children), argument);
                        }
                    }
                }
                else{
                    ObjectComponent** resultLoc = (ObjectComponent**)resultstore;

                    for(auto i = StartingPoint->begin(); i != StartingPoint->end(); i++){
                        if((*i)->type == *((ObjectComponentType*)argument)){
                            *resultLoc = (*i);

                            return 0;
                        }

                        if((*i)->Children.size() > 0){
                            if(!RecuseSearchChild(Setting, multi, resultstore, &((*i)->Children), argument)){
                                return 0;
                            }
                        }
                    }
                }
                return 0;
            }
        case 1: // name
            {    
                if(!argument){
                    FAULT("Missing Comparitor Argument");
                    return -1;
                }

                if(multi){
                    std::list<ObjectComponent*>* resultLoc = (std::list<ObjectComponent*>*)resultstore;

                    for(auto i = StartingPoint->begin(); i != StartingPoint->end(); i++){
                        if((*i)->type == OBJECT_GAME_OBJECT){
                            GameObject* Gobj = (GameObject*)(*i);

                            if(Gobj->Name == *((std::string*)argument)){
                                resultLoc->push_back(*i);
                            }
                        }

                        if((*i)->Children.size() > 0){
                            RecuseSearchChild(Setting, multi, resultstore, &((*i)->Children), argument);
                        }
                    }
                }
                else{
                    ObjectComponent** resultLoc = (ObjectComponent**)resultstore;

                    for(auto i = StartingPoint->begin(); i != StartingPoint->end(); i++){
                        if((*i)->type == OBJECT_GAME_OBJECT){
                            GameObject* Gobj = (GameObject*)(*i);

                            if(Gobj->Name == *((std::string*)argument)){
                                *resultLoc = (*i);
                            }

                            return 0;
                        }

                        if((*i)->Children.size() > 0){
                            if(!RecuseSearchChild(Setting, multi, resultstore, &((*i)->Children), argument)){
                                return 0;
                            }
                        }
                    }
                }
                return 0;
            }
        case 2: // tag
            {
                if(!argument){
                    FAULT("Missing Comparitor Argument");
                    return -1;
                }

                if(multi){
                    std::list<ObjectComponent*>* resultLoc = (std::list<ObjectComponent*>*)resultstore;

                    for(auto i = StartingPoint->begin(); i != StartingPoint->end(); i++){
                        if((*i)->type == OBJECT_GAME_OBJECT){
                            GameObject* Gobj = (GameObject*)(*i);

                            if(Gobj->Tag == *((std::string*)argument)){
                                resultLoc->push_back(*i);
                            }
                        }

                        if((*i)->Children.size() > 0){
                            RecuseSearchChild(Setting, multi, resultstore, &((*i)->Children), argument);
                        }
                    }

                    return 0;
                }
                else{
                    ObjectComponent** resultLoc = (ObjectComponent**)resultstore;

                    for(auto i = StartingPoint->begin(); i != StartingPoint->end(); i++){
                        if((*i)->type == OBJECT_GAME_OBJECT){
                            GameObject* Gobj = (GameObject*)(*i);

                            if(Gobj->Tag == *((std::string*)argument)){
                                *resultLoc = (*i);
                            }

                            return 0;
                        }

                        if((*i)->Children.size() > 0){
                            if(!RecuseSearchChild(Setting, multi, resultstore, &((*i)->Children), argument)){
                                return 0;
                            }
                        }
                    }
                }
                return 0;
            }
        case 3: // MainCam (SingularOnly)
            {
                Camera** resultLoc = (Camera**)resultstore;

                for(auto i = StartingPoint->begin(); i != StartingPoint->end(); i++){
                    if((*i)->type == OBJECT_CAMERA_OBJECT){
                        *resultLoc = (Camera*)(*i);
                        return 0;
                    }

                    if((*i)->Children.size() > 0){
                        if(!RecuseSearchChild(Setting, multi, resultstore, &((*i)->Children))){
                            return 0;
                        }
                    }
                }
                return -1;
            }
        
        default: //Error
            FAULT("Invalid Argument Provided");
            return -1;
            break;
        }

        return -1;
    }

    ObjectComponent* GameInstance::GetObjectOfType(ObjectComponentType type){
        ObjectComponent* Result = nullptr;

        RecuseSearchChild(0, false, (void*)(&Result), &this->objects, (void*)(&type));
        
        return Result;
    }
    std::list<ObjectComponent*> GameInstance::GetObjectsOfType(ObjectComponentType type){
        std::list<ObjectComponent*> Result = {};

        RecuseSearchChild(0, true, (void*)(&Result), &this->objects, (void*)(&type));

        return Result;
    }
    GameObject* GameInstance::GetGameObjectWithName(std::string name){
        GameObject* Result = nullptr;

        RecuseSearchChild(1, false, (void*)(&Result), &this->objects, (void*)(&name));

        return Result;
    }
    GameObject* GameInstance::GetGameObjectWithTag(std::string tag){
        GameObject* Result = nullptr;

        RecuseSearchChild(2, false, (void*)(&Result), &this->objects, (void*)(&tag));

        return Result;
    }
    std::list<GameObject*> GameInstance::GetGameObjectsWithName(std::string name){
        std::list<GameObject*> Result = {};

        RecuseSearchChild(1, true, (void*)(&Result), &this->objects, (void*)(&name));

        return Result;
    }
    std::list<GameObject*> GameInstance::GetGameObjectsWithTag(std::string tag){
        std::list<GameObject*> Result = {};

        RecuseSearchChild(2, true, (void*)(&Result), &this->objects, (void*)(&tag));

        return Result;
    }

    Camera* PriorCamera = nullptr;

    Camera* GameInstance::GetMainCamera(){
        Camera* Result = nullptr;

        if(!PriorCamera){
            RecuseSearchChild(3, false, (void*)(&Result), &this->objects);

            PriorCamera = Result;
        }
        else{
            Result = PriorCamera;
        }

        return Result;
    }

    int instantiate(ObjectComponent* Object){
        __GAME__GLOBAL__INSTANCE__->objects.push_back(Object);

        return 0;
    }
    int destroy(ObjectComponent* Object){
        __GAME__GLOBAL__INSTANCE__->objects.remove(Object);

        return 0;
    }

} // namespace UnifiedEngine