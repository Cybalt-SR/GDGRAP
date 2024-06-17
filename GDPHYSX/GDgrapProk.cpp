#define STB_IMAGE_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION

#include <GD_Graphics/util.h>
#include <GD_Graphics/Shader.h>
#include <GD_Graphics/Texture.h>
#include <GD_Graphics/Window.h>
#include <GD_Graphics/Skybox.h>
#include <GD_Graphics/Light.h>
#include <GD_Graphics/Framebuffer.h>
#include <GD_Graphics/RenderPipeline.h>
#include <GD_Graphics/Mesh.h>

#include <GD_Engine/Input/Implementations/Implementations.h>
#include <GD_Engine/Input/InputSystem.h>
#include <GD_Engine/ObjectHandlers/ObjectHandlers.h>
#include <GD_Engine/Objects/Objects.h>
#include <GD_Engine/ObjectFunctions/ObjectFunctions.h>
#include <GD_Engine/Datatypes/Vectors.h>
#include <GD_Engine/Time.h>

using namespace gde;

int main(void)
{
    //PHASE 1 CODE
    int particles_to_spawn = 0;
    std::cout << "Enter particle count: ";
    std::cin >> particles_to_spawn;

    auto mTime = new Time();

    /* Initialize GLFW*/
    if (!glfwInit())
        return -1;

    Window* mWindow = new Window("PHASE 1 GDPHYSX", 900, 900);
    
    /* Initialize GLAD*/
    gladLoadGL();

#pragma region Rendering Requisites Setup
    //Shaders setup
    auto depthShader = new Shader("Shaders/object.vert", "Shaders/depth.frag");
    auto litShader = new Shader("Shaders/object.vert", "Shaders/lit.frag");
    auto unlitShader = new Shader("Shaders/object.vert", "Shaders/unlit.frag");
    auto Cam3rdPPShader = new Shader("Shaders/camshader.vert", "Shaders/camshader.frag");
    auto Cam1stPPShader = new Shader("Shaders/camshader.vert", "Shaders/camshader.frag");
    
    auto CamOrthoPPShader = new Shader("Shaders/camshader.vert", "Shaders/camshader.frag");
    glUseProgram(CamOrthoPPShader->shaderID);
    glUniform1f(glGetUniformLocation(CamOrthoPPShader->shaderID, "saturation"), 1.0f);
    glUniform4fv(glGetUniformLocation(CamOrthoPPShader->shaderID, "tint"), 1, glm::value_ptr(glm::vec4(1, 1, 1, 1)));

    //RenderPipeline setup
    Camera* active_camera = nullptr;
    auto mRenderPipeline = new RenderPipeline(glm::vec2(mWindow->win_x, mWindow->win_y));
#pragma endregion

#pragma region Input
    auto mInputSystem = new InputSystem();
    auto player_name = "MAIN";
    mInputSystem->RegisterActionListener(player_name, new MouseRightDragImplementation());
    mInputSystem->RegisterActionListener(player_name, new KeyPressImplementation<GLFW_KEY_Q>());
    mInputSystem->RegisterActionListener(player_name, new KeyPressImplementation<GLFW_KEY_SPACE>());
    mInputSystem->RegisterActionListener(player_name, new WasdDeltaImplementation());
#pragma endregion

#pragma region Object setup
    //Object handlers setup
    auto mPhysicsHandler = new PhysicsHandler();
    auto mInputHandler = new InputHandler();
    auto mEarlyUpdate = new ObjectHandler<EarlyUpdate>();
    auto mUpdate = new ObjectHandler<Update>();
    auto mLateUpdate = new ObjectHandler<LateUpdate>();
    //root
    auto root_object = new Root();
    root_object->RegisterHandler(mPhysicsHandler);
    root_object->RegisterHandler(mInputHandler);
    root_object->RegisterHandler(mEarlyUpdate);
    root_object->RegisterHandler(mUpdate);
    root_object->RegisterHandler(mLateUpdate);

    //Camera setup
    auto player_input = new InputPlayer(player_name);
    player_input->SetParent(root_object);
    auto camera_dolly = new OrbitalControl();
    camera_dolly->SetParent(player_input);

    auto mPerspectiveCam = new PerspectiveCamera(mWindow, CamOrthoPPShader);
    mPerspectiveCam->angles = 90;
    mPerspectiveCam->farClip = 1000.0f;
    mPerspectiveCam->WorldUp = Vector3(0, 1, 0);
    mPerspectiveCam->TranslateWorld(Vector3(0, 0, -400));
    mPerspectiveCam->SetParent(camera_dolly);
    auto mOrthoCam = new OrthographicCamera(mWindow, CamOrthoPPShader);
    mOrthoCam->orthoRange = 450;
    mOrthoCam->farClip = 1000.0f;
    mOrthoCam->WorldUp = Vector3(0, 1, 0);
    mOrthoCam->TranslateWorld(Vector3(0, 0, -400));
    mOrthoCam->SetParent(camera_dolly);
    active_camera = mPerspectiveCam;

    auto mCamSwitcher = new CameraSwitcher((void*)mPerspectiveCam, (void*)mOrthoCam, (void**)&active_camera);
    mCamSwitcher->SetParent(player_input);

    auto mPauser = new SpacebarPauser(mTime);
    mPauser->SetParent(player_input);

    //Mesh and material caching
    auto sphere_mesh = new Mesh("3D/sphere.obj");

    auto create_unlit_rendercall = [sphere_mesh, unlitShader, mRenderPipeline](Vector3 color) {
        auto unlit_mat = new Material(unlitShader);
        unlit_mat->setOverride<glm::vec3>("color", color);
        auto new_drawcall = new DrawCall(sphere_mesh, unlit_mat);
        mRenderPipeline->RegisterDrawCall(new_drawcall);
        return new_drawcall;
    };

    std::vector<DrawCall*> drawcalls = {
        create_unlit_rendercall(Vector3(1, 0, 0)),
        create_unlit_rendercall(Vector3(0, 1, 0)),
        create_unlit_rendercall(Vector3(0, 0, 1)),
        create_unlit_rendercall(Vector3(1, 1, 0)),
        create_unlit_rendercall(Vector3(1, 0, 1)),
        create_unlit_rendercall(Vector3(0, 1, 1)),
    };
    //reference sphere renderobject setup

    auto CreateParticleFunction = [drawcalls, root_object, unlitShader, mRenderPipeline]() {
        //sphere rigidobject setup
        auto sphere_rigidobject = new RigidObject();
        sphere_rigidobject->damping = 1.0f;

        //sphere renderobject setup
        auto sphere_renderobject = new RenderObject(drawcalls[rand() % drawcalls.size()]);
        sphere_renderobject->Scale(Vector3(1, 1, 1) * 0.6f);
        sphere_renderobject->SetParent(sphere_rigidobject);

        return sphere_rigidobject;
    };

    //particle system setup
    auto system = new ParticleSystem(CreateParticleFunction);
    system->spawns_per_sec = 0;
    system->start_force.random_between_two = true;
    system->start_force.valueA = Vector3(-1, 0.5, -1) * 800;
    system->start_force.valueB = Vector3(1, 4, 1) * 800;
    system->start_lifetime.random_between_two = true;
    system->start_lifetime.valueA = 0.4f;
    system->start_lifetime.valueA = 15;
    system->SetParent(root_object);

    //physics force setup
    auto gravity_volume = new ForceVolume();
    gravity_volume->shape = ForceVolume::GLOBAL;
    gravity_volume->mode = ForceVolume::DIRECTIONAL;
    gravity_volume->vector = Vector3(0, -10, 0);
    gravity_volume->forceMode = ForceVolume::VELOCITY;
    gravity_volume->SetParent(root_object);

    //PHASE 1 CODE
    system->Spawn(particles_to_spawn);

#pragma endregion

    /// MAIN GAME LOOP
    while (!glfwWindowShouldClose(mWindow->window))
    {
        //Update input system
        mInputSystem->UpdateStates([mInputHandler](std::string name, gde::input::InputAction* action, bool changed) {
            for (auto input_player : mInputHandler->object_list) {
                if (input_player->get_player_name() != name)
                    continue;

                for (auto input_customer : input_player->inputhandler.object_list)
                    input_customer->TryReceive(action, changed);
            }
        }, mWindow);

        /* Poll for and process events */
        glfwPollEvents();

        //Early update
        for (auto updatable : mEarlyUpdate->object_list)
        {
            updatable->InvokeEarlyUpdate();
        }

        //Update Render pipeline
        mRenderPipeline->RenderFrame(active_camera->World()->position, active_camera->GetViewMat(), active_camera->getproj(), active_camera->mShader);
        //Render window
        glfwSwapBuffers(mWindow->window);

        //Update other handlers
        mTime->TickFixed([mPhysicsHandler, mUpdate, mLateUpdate, root_object](double deltatime) {
            mPhysicsHandler->Update(deltatime);

            //Normal Update
            for (auto updatable : mUpdate->object_list)
            {
                updatable->InvokeUpdate(deltatime);
            }
            //Late Update
            for (auto updatable : mLateUpdate->object_list)
            {
                updatable->InvokeLateUpdate(deltatime);
            }
        });

        //Delete all queued for deletions
        std::list<Object*> toDelete;

        root_object->CallRecursively([&toDelete](Object* object) {
            if (object->get_isDestroyed()) {
                toDelete.push_back(object);
            }
        });

        for (auto deletee : toDelete)
        {
            delete deletee;
        }
    }
    
    mRenderPipeline->CleanUp();

    glfwTerminate();
}