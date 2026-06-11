#include <glutil/glutil.hpp>

#include "config.hpp"
#include "gameloop.hpp"
#include "mesh.hpp"
#include "manager.hpp"
#include "component.hpp"

#include <glm/glm.hpp>
#include <iostream>

#define NOMINMAX
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

int main() {
    auto& gc = GraphicsContext::Get();
    bool ok = gc.Init(1920, 1080, "AIMLAB");
    if (!ok)
        return -1;

    InputManager::Get().Init();

    glutil::GLProgram program = glutil::ShaderLoader::loadProgramToGL(PROJECT_ROOT / "shader" / "aimlab.vert",
                                                                      PROJECT_ROOT / "shader" / "aimlab.frag");
    if (!program.ok) {
        std::cout << program.error;
        return -1;
    }
    gc.SetProgram(program.id);

    auto& rm = ResourceManager::Get();
    rm.SetDefaultTexture(ASSET_ROOT / "basic" / "texture" / "default.bmp");

    rm.AddMesh("plane", ASSET_ROOT / "basic" / "model" / "plane.obj");
    rm.AddCpuMesh("plane", ASSET_ROOT / "basic" / "model" / "plane.obj");
    rm.AddMesh("cube",  ASSET_ROOT / "basic" / "model" / "cube.obj");
    Mesh* planeMesh = rm.GetMesh("plane");
    
    // 카메라
    Camera& camera = Camera::Get();
    camera.position = glm::vec3(0.f, 1.f, 15.f);
    camera.up = glm::vec3(0.f, 1.f, 0.f);
    camera.yaw = -90.f;
    camera.pitch = 0.f;
    camera.speed = 5.f;

    GameLoop gEngine;

    // System Controller
    GameObject* system = new GameObject();
    system->AddComponent(new SystemController());
    gEngine.system.push_back(system);

    // Camera Object
    GameObject* cameraObject = new GameObject();
    auto* cameraController = new CameraController();
    cameraObject->AddComponent(cameraController);
    auto* weaponsystem = new WeaponSystem();
    weaponsystem->targets = &gEngine.world3d;
    weaponsystem->decalsToSpawn = &gEngine.decalsToSpawn;
    cameraObject->AddComponent(weaponsystem);
    gEngine.system.push_back(cameraObject);

    rm.AddTexture("bulletHole", ASSET_ROOT / "AIMLAB" / "texture" / "bulletHole.png");
    // 바닥
    GameObject* plane = new GameObject();
    plane->type = EObjectType::Block;
    plane->transform.rotation.y = 180.f;
    plane->transform.scale = glm::vec3(20.f, 1.f, 20.f);

    auto* planeRenderer = new MeshRenderer(planeMesh, new Material(rm.AddTexture("grid", ASSET_ROOT / "basic" / "texture" / "grid.bmp")));
    plane->AddComponent(planeRenderer);
    gEngine.world3d.push_back(plane);

    // 벽
    GameObject* plane2 = new GameObject();
    plane2->type = EObjectType::Block;
    plane2->transform.rotation.x = 90.f;
    plane2->transform.position.z = -20.f;
    plane2->transform.position.y = 20.f;

    plane2->transform.scale = glm::vec3(20.f, 1.f, 20.f);

    auto* planeRenderer2 = new MeshRenderer(planeMesh, new Material(rm.AddTexture("backwall", ASSET_ROOT / "AIMLAB" / "texture" / "container.jpg")));
    plane2->AddComponent(planeRenderer2);
    gEngine.world3d.push_back(plane2);



    // Target Spawner + Particle
    rm.AddMesh("target", ASSET_ROOT / "AIMLAB" / "model" / "target.obj");
    rm.AddCpuMesh("target", ASSET_ROOT / "AIMLAB" / "model" / "target.obj");
    rm.AddTexture("target", ASSET_ROOT / "AIMLAB" / "texture" / "target.png");
  
    rm.AddMesh("particle", ASSET_ROOT / "basic" / "model" / "sphere.obj");
    rm.AddTexture("particle", ASSET_ROOT / "AIMLAB" / "texture" / "spark.png");
    
    GameObject* TargetSpawner = new GameObject();
    TargetSpawner->transform.position = glm::vec3(0.f, 0.f, 0.f);
    auto* spawner = new TargetSpawnerComponent();
    spawner->targetsToSpawn = &gEngine.targetsToSpawn;
    spawner->world3d = &gEngine.world3d;
    TargetSpawner->AddComponent(spawner);
    gEngine.system.push_back(TargetSpawner);

    // Round Timer
    auto* timerObj = new GameObject();
    auto* roundTimer = new RoundTimerComponent();
    timerObj->AddComponent(roundTimer);
    gEngine.system.push_back(timerObj);

    // Gun
    GameObject* gun = new GameObject();
    gun->transform.scale = glm::vec3(0.006f);
    gun->AddComponent(new GunController(weaponsystem, {0.35f, -0.4f, 0.8f}));
    gun->AddComponent(new ViewModelRenderer(
      rm.AddMesh("gun", ASSET_ROOT / "ak47" / "ak47.obj"),
      new Material(rm.AddTexture("gun", ASSET_ROOT / "ak47" / "123456_wire_115115115_color.png"))));
    gEngine.world2d.push_back(gun);

    // Crosshair
    GameObject* crosshair = new GameObject();
    CrossHairComponent* chc = new CrossHairComponent();
    crosshair->AddComponent(chc);
    crosshair->AddComponent(new OrthogonalRenderer(rm.AddMesh("crosshair", ASSET_ROOT / "AIMLAB" / "model" / "crosshair.obj"), chc->material));
    gEngine.world2d.push_back(crosshair);



    // 숫자, 특수문자 저장
    for (int i = 0; i < 10; i++) rm.AddTexture("num" + std::to_string(i), ASSET_ROOT / "num" / (std::to_string(i) + ".png"));
    rm.AddTexture("numColon", ASSET_ROOT / "num" / "colon.png");
    rm.AddTexture("numPercent", ASSET_ROOT / "num" / "percent.png");
    rm.AddTexture("numSlash", ASSET_ROOT / "num" / "slash.png");

    // 글자 저장
    rm.AddTexture("labelScore", ASSET_ROOT / "num" / "score.png");
    rm.AddTexture("labelAccuracy", ASSET_ROOT / "num" / "accuracy.png");
    rm.AddTexture("labelAmmo", ASSET_ROOT / "num" / "ammo.png");
    rm.AddTexture("start", ASSET_ROOT / "num" / "start.png");
    rm.AddTexture("restart", ASSET_ROOT / "num" / "restart.png");

    // HUD 생성
    auto* hudObj = new GameObject();
    auto* hud = new HUDComponent(&gEngine.world2d, weaponsystem, roundTimer);
    float s = hud->digitSize;

    // 시간
    hud->timeDigits.push_back(hud->AddDigit(0.5f, 1.f, -s * 2.f, -s));
    hud->timeDigits.push_back(hud->AddDigit(0.5f, 1.f, -s, -s));
    hud->AddSymbol("numColon", 0.5f, 1.f, -s * 0.25f, -s);
    hud->timeDigits.push_back(hud->AddDigit(0.5f, 1.f, s * 0.50f, -s));
    hud->timeDigits.push_back(hud->AddDigit(0.5f, 1.f, s * 1.50f, -s));

    // 점수
    for (int i = 0; i < 5; i++)
        hud->scoreDigits.push_back(hud->AddDigit(1.f, 1.f, -s * (5 - i), -s));

    // 정확도
    for (int i = 0; i < 3; i++)
        hud->accuracyDigits.push_back(hud->AddDigit(1.f, 1.f, -s * (4 - i), -s * 2.f));
    hud->AddSymbol("numPercent", 1.f, 1.f, -s, -s * 2.f);

    // 탄약
    hud->ammoDigits.push_back(hud->AddDigit(1.f, 1.f, -s * 4.f, -s * 3.f));
    hud->ammoDigits.push_back(hud->AddDigit(1.f, 1.f, -s * 3.f, -s * 3.f));
    hud->AddSymbol("numSlash", 1.f, 1.f, -s * 2.5f, -s * 3.f);
    hud->ammoDigits.push_back(hud->AddDigit(1.f, 1.f, -s * 2.f, -s * 3.f));
    hud->ammoDigits.push_back(hud->AddDigit(1.f, 1.f, -s, -s * 3.f));

    hud->AddSymbol("labelScore", 1.f, 1.f, -s * 5.f - 210.f, -s * 0.7f, 98.f, s);
    hud->AddSymbol("labelAccuracy", 1.f, 1.f, -s * 4.f - 205.f, -s * 2.f, 152.f, s);
    hud->AddSymbol("labelAmmo", 1.f, 1.f, -s * 5.f - 200.f, -s * 3.f, 108.f, s);

    // 감도 
    hud->auxDisplayDigits.push_back(hud->AddDigit(1.f, 1.f, -s * 3.f, -s * 4.f));
    hud->auxDisplayDigits.push_back(hud->AddDigit(1.f, 1.f, -s * 2.f, -s * 4.f));
    hud->auxDisplayDigits.push_back(hud->AddDigit(1.f, 1.f, -s ,      -s * 4.f));

    // 카운트 다운
    hud->countdownDigit = (hud->AddDigit(0.5f, 0.5f, 0.f, 0.f));
    hud->countdownDigit->visible = false;

    // 문구
    hud->startSymbol = hud->AddSymbol("start", 0.5f, 0.0f, 0.f, 70.f, 500.f, 40.f);
    hud->restartSymbol = hud->AddSymbol("restart", 0.5f, 0.0f, 0.f, 70.f, 500.f, 40.f);

    hudObj->AddComponent(hud);
    gEngine.system.push_back(hudObj);

    GameObject* gm = new GameObject();
    GameManagerComponent* gmc = new GameManagerComponent(roundTimer, weaponsystem, &gEngine.world3d);
    gm->AddComponent(gmc);
    gEngine.system.push_back(gm);



#ifdef AIMLAB_OPTION_GL_DEBUG
    glutil::debug::snapshot(true).bufferVAOInfo(true, true, true).capture();
#endif

    gEngine.Run();

    return 0;
}