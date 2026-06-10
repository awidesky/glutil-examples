#include "config.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <component.hpp>
#include <mesh.hpp>
#include <manager.hpp>


CrossHairComponent::CrossHairComponent() {
    material = new Material(
      {ResourceManager::Get().AddTexture("crosshair1", ASSET_ROOT / "crosshair" / "crosshair_cross.png"),
       ResourceManager::Get().AddTexture("crosshair2", ASSET_ROOT / "crosshair" / "crosshair_dot.png"),
       ResourceManager::Get().AddTexture("crosshair3", ASSET_ROOT / "crosshair" / "crosshair_dot_small.png"),
       ResourceManager::Get().AddTexture("crosshair4", ASSET_ROOT / "crosshair" / "crosshair_circle_dot.png"),
       ResourceManager::Get().AddTexture("crosshair5", ASSET_ROOT / "crosshair" / "crosshair_circle_dot_small.png")});
}

void CrossHairComponent::Start() { pOwner->transform.scale = glm::vec3(32.0f, 32.0f, 1.0f); }

void CrossHairComponent::Input() {
    if (InputManager::Get().IsKeyDown(GLFW_KEY_1)) material->useTexture(0);
    else if (InputManager::Get().IsKeyDown(GLFW_KEY_2)) material->useTexture(1);
    else if (InputManager::Get().IsKeyDown(GLFW_KEY_3)) material->useTexture(2);
    else if (InputManager::Get().IsKeyDown(GLFW_KEY_4)) material->useTexture(3);
    else if (InputManager::Get().IsKeyDown(GLFW_KEY_5)) material->useTexture(4);
}

void CrossHairComponent::Update(float dt) {
    (void)dt;
    auto& gc = GraphicsContext::Get();

    int w, h;
    gc.GetWindowSize(w, h);

    pOwner->transform.position.x = w * 0.5f;
    pOwner->transform.position.y = h * 0.5f;
    pOwner->transform.position.z = 0.0f;
}

NumberController::NumberController(Material* mat, float ancX, float ancY, float offX, float offY, float w, float h)
    : material(mat), anchorX(ancX), anchorY(ancY), offsetX(offX), offsetY(offY), width(w), height(h) {}

void NumberController::Start() { pOwner->transform.scale = glm::vec3(width, height, 1.f); }

void NumberController::SetDigit(int d) {
    digit = std::clamp(d, 0, 9);
    if (material) material->useTexture((size_t)digit);
}

void NumberController::Update(float dt) {
    (void)dt;
    if (!visible) {
        pOwner->transform.scale = glm::vec3(0.f);
        return;
    }
    pOwner->transform.scale = glm::vec3(width, height, 1.f);
    int w, h;
    GraphicsContext::Get().GetWindowSize(w, h);
    pOwner->transform.position.x = w * anchorX + offsetX;
    pOwner->transform.position.y = h * anchorY + offsetY;
}

void RoundTimerComponent::StartRound(float d) {
    duration = remainTime = d;
    isRunning = true;
}

void RoundTimerComponent::Update(float dt) {
    if (isRunning) remainTime -= dt;
}

bool RoundTimerComponent::IsExpired() const { return isRunning && remainTime <= 0.f; }

void RoundTimerComponent::Reset() {
    remainTime = duration = 0.f;
    isRunning = false;
    // TODO : ScoreManager::Get().Reset()도 여기서 같이 해도 될듯?
    // TODO : world의 타겟들 다 지우는 거
}

void WeaponSystem::Fire()
{
    if (isReloading || !targets || fireCooldown > 0.f) return;
    if (currentAmmo <= 0) {
        Reload();
        return;
    }
    currentAmmo--;
    fireCooldown = fireInterval;

    const Camera& camera = Camera::Get();
    PhysicsSystem::Ray ray = {camera.position, camera.GetForward()};
    TargetObject* nearestHitTarget = NULL;
    float minHitDist = FLT_MAX;
    glm::vec3 nearestHitPoint;
    glm::vec3 hitPoint, hitNormal;
    CpuMesh* targetMesh = ResourceManager::Get().GetCpuMesh("target");
    
    std::vector<GameObject*> blocks;
    CpuMesh* planeMesh = ResourceManager::Get().GetCpuMesh("plane");


    for (auto* obj : *targets) {
        if (obj->state != EObjectState::Spawned) continue;
        if (obj->type == EObjectType::Block) {
            blocks.push_back(obj);
            continue;
        }
        auto* target = dynamic_cast<TargetObject*>(obj);
        if (!target || target->state != EObjectState::Spawned) continue;

        if (PhysicsSystem::Get().RaySphereIntersect(ray, target->transform.position, target->radius)) {
            if (PhysicsSystem::Get().RayMeshIntersect(ray, *targetMesh, target->transform.GetWorldMatrix(), hitPoint,
                                                      hitNormal)) {
                float dist = glm::distance(camera.position, target->transform.position);
                if (dist < minHitDist) {
                    nearestHitTarget = target;
                    minHitDist = dist;
                    nearestHitPoint = hitPoint;
                }
            }
        }
    }
    if (nearestHitTarget != nullptr) {
        ScoreManager::Get().RecordHit(nearestHitTarget->GetReactionTime());
        nearestHitTarget->OnHit(nearestHitPoint);
    } else {
        ScoreManager::Get().RecordMiss();
        // 타겟과 충돌 X 이면 벽이나 바닥과 충돌했는지 확인
        for (auto block : blocks) {
            if (PhysicsSystem::Get().RayMeshIntersect(ray, *planeMesh, block->transform.GetWorldMatrix(), hitPoint, hitNormal)) {
                GameObject* gunDecal = new GameObject();
                DecalComponent* decal = new DecalComponent();
                gunDecal->transform.rotation = block->transform.rotation;
                gunDecal->transform.position = hitPoint + hitNormal * floatingAmount;
                floatingAmount += 0.001f;
                gunDecal->AddComponent(new MeshRenderer(ResourceManager::Get().GetMesh("plane"),
                                                        new Material(ResourceManager::Get().GetTexture("target"))));
                gunDecal->AddComponent(decal);
                decalsToSpawn->push_back(gunDecal);
            }
        }
    }
    for (auto* fireListener : fireListeners)
        fireListener->Fire();
}


void TargetSpawnerComponent::Update(float dt) {
    if (!targetsToSpawn) return;
    if (GameStateManager::Get().state == EGameState::CountDown || GameStateManager::Get().state == EGameState::Result)
        return;

    spawnInterval = (float)rand() / RAND_MAX * 2.0f + 1.0f;
    spawnTimer += dt;
    if (spawnTimer < spawnInterval) return;
    spawnTimer = 0.f;

    int alive = 0;
    for (auto* obj : *world3d)
        if (obj->state == EObjectState::Spawned && dynamic_cast<TargetObject*>(obj)) alive++;

    if (alive >= maxTargets) return;

    spawnAmount = rand() % 3 + 1;
    for (int i = 0; i < spawnAmount; ++i)
        SpawnTarget();
}

void TargetSpawnerComponent::SpawnTarget() {
    float x = ((float)rand() / RAND_MAX * 2.f - 1.f) * 9.5f;
    float z = round(-(float)rand() / RAND_MAX * 10.f);

    TargetObject* target = new TargetObject();
    target->type = EObjectType::Target;
    target->transform.position = glm::vec3(x, 0.0f, z);
    target->transform.scale = glm::vec3(0.2f, 0.2f, 0.05f);
    target->transform.rotation = glm::vec3(-90.f, 0.f, 0.f);

    Mesh* mesh = ResourceManager::Get().GetMesh("target");
    Texture* tex = ResourceManager::Get().GetTexture("target");

    target->AddComponent(new MeshRenderer(mesh, new Material(tex)));
    target->AddComponent(new TargetController());
    target->onHitEmit = [](glm::vec3 pos, glm::vec3 dir) { ParticleSystem::Get().Emit(pos, dir, 20); };

    targetsToSpawn->push_back(target);
}

HUDComponent::HUDComponent(std::vector<GameObject*>* world2d, WeaponSystem* ws, RoundTimerComponent* rt)
    : world2d(world2d), weaponSystem(ws), roundTimer(rt) {}

NumberController* HUDComponent::AddDigit(float ancX, float ancY, float offX, float offY) {
    auto& rm = ResourceManager::Get();
    auto* mat =
      new Material({rm.GetTexture("num0"), rm.GetTexture("num1"), rm.GetTexture("num2"), rm.GetTexture("num3"),
                    rm.GetTexture("num4"), rm.GetTexture("num5"), rm.GetTexture("num6"), rm.GetTexture("num7"),
                    rm.GetTexture("num8"), rm.GetTexture("num9")});
    auto* obj = new GameObject();
    auto* ctrl = new NumberController(mat, ancX, ancY, offX, offY, digitSize, digitSize);
    obj->AddComponent(ctrl);
    obj->AddComponent(new OrthogonalRenderer(rm.GetMesh("crosshair"), mat));
    world2d->push_back(obj);
    return ctrl;
}

NumberController* HUDComponent::AddSymbol(const std::string& texName, float ancX, float ancY, float offX, float offY) {
    auto& rm = ResourceManager::Get();
    auto* mat = new Material(rm.GetTexture(texName));
    auto* obj = new GameObject();
    auto* ctrl = new NumberController(mat, ancX, ancY, offX, offY, digitSize * 0.5f, digitSize * 0.5f);
    obj->AddComponent(ctrl);
    obj->AddComponent(new OrthogonalRenderer(rm.GetMesh("crosshair"), mat));
    world2d->push_back(obj);
    return ctrl;
}

NumberController* HUDComponent::AddSymbol(const std::string& texName, float ancX, float ancY, float offX, float offY,
                                          float w, float h) {
    auto& rm = ResourceManager::Get();
    auto* mat = new Material(rm.GetTexture(texName));
    auto* obj = new GameObject();
    auto* ctrl = new NumberController(mat, ancX, ancY, offX, offY, w, h);
    obj->AddComponent(ctrl);
    obj->AddComponent(new OrthogonalRenderer(rm.GetMesh("crosshair"), mat));
    world2d->push_back(obj);
    return ctrl;
}

void HUDComponent::Update(float dt) {
    (void)dt;
    // 카운트 다운
    if (countdownDigit) {
        countdownDigit->visible = (GameStateManager::Get().state == EGameState::CountDown);
        countdownDigit->SetDigit((int)std::ceil(GameStateManager::Get().countdownTimer));
    }

    if (startSymbol) startSymbol->visible = GameStateManager::Get().state == EGameState::Waiting;
    if (restartSymbol) restartSymbol->visible = GameStateManager::Get().state == EGameState::Result;

    // 탄약
    if (weaponSystem && ammoDigits.size() >= 4) {
        int cur = weaponSystem->currentAmmo;
        int max = weaponSystem->maxAmmo;
        ammoDigits[0]->SetDigit(cur / 10);
        ammoDigits[1]->SetDigit(cur % 10);
        ammoDigits[2]->SetDigit(max / 10);
        ammoDigits[3]->SetDigit(max % 10);
    }

    // 시간
    if (roundTimer && timeDigits.size() >= 4) {
        int remain = (int)std::max(0.f, roundTimer->remainTime);
        int mm = remain / 60;
        int ss = remain % 60;
        timeDigits[0]->SetDigit(mm / 10);
        timeDigits[1]->SetDigit(mm % 10);
        timeDigits[2]->SetDigit(ss / 10);
        timeDigits[3]->SetDigit(ss % 10);
    }

    if (GameStateManager::Get().state != EGameState::Playing) return;

    // 점수
    {
        int score = ScoreManager::Get().score;
        for (int i = (int)scoreDigits.size() - 1; i >= 0; i--) {
            scoreDigits[i]->SetDigit(score % 10);
            score /= 10;
        }
    }

    // 정확도
    {
        int acc = (int)ScoreManager::Get().GetAccuracy();
        for (int i = (int)accuracyDigits.size() - 1; i >= 0; i--) {
            accuracyDigits[i]->SetDigit(acc % 10);
            acc /= 10;
        }
    }
}

GameManagerComponent::GameManagerComponent(RoundTimerComponent* rt, WeaponSystem* ws, std::vector<GameObject*>* w)
    : roundTimer(rt), weaponSystem(ws), world3d(w) {}

void GameManagerComponent::Update(float dt) {

    if (InputManager::Get().IsKeyDown(GLFW_KEY_G) && !m_prevG)
        if (GameStateManager::Get().state == EGameState::Waiting ||
            GameStateManager::Get().state == EGameState::Result) {
            StartCountDown();
        }

    m_prevG = InputManager::Get().IsKeyDown(GLFW_KEY_G);

    if (InputManager::Get().IsKeyDown(GLFW_KEY_H) && !m_prevH) {
        EndRound();
    }

    m_prevH = InputManager::Get().IsKeyDown(GLFW_KEY_H);

    if (GameStateManager::Get().state == EGameState::Playing && roundTimer->IsExpired()) {
        EndRound();
    }
    if (GameStateManager::Get().state == EGameState::CountDown) {
        GameStateManager::Get().countdownTimer -= dt;
        if (GameStateManager::Get().countdownTimer <= 0.f) {
            StartRound();
        }
    }
}

void GameManagerComponent::StartRound() {

    GameStateManager::Get().StartRound();
    ScoreManager::Get().Reset();
    if (roundTimer) roundTimer->StartRound(10.f);
    if (weaponSystem) {
        weaponSystem->StartRound();
    }
}

void GameManagerComponent::EndRound() {

    GameStateManager::Get().EndRound();
    if (roundTimer) roundTimer->Reset();
    for (auto* obj : *world3d) {
        if (dynamic_cast<TargetObject*>(obj)) {
            obj->state = EObjectState::Dying;
        }
    }
}

void GameManagerComponent::StartCountDown() {

    GameStateManager::Get().StartCountDown();
    for (auto* obj : *world3d) {
        if (dynamic_cast<TargetObject*>(obj)) {
            obj->state = EObjectState::Dying;
        }
    }
}