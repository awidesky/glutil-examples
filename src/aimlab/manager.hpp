#ifndef AIMLAB_MANAGER_HPP
#define AIMLAB_MANAGER_HPP


#include <vector>
#include <cstdlib>
#include <cstdio>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <config.hpp>

#include "component.hpp"
#include "mesh.hpp"
#include "score.hpp"


enum class EGameState { Waiting, Playing, Result, CountDown };

class GameStateManager {
public:
    float countdownTimer = 3.f;
    static GameStateManager& Get() {
        static GameStateManager state;
        return state;
    }
    GameStateManager(const GameStateManager&) = delete;
    GameStateManager& operator=(const GameStateManager&) = delete;

    EGameState state = EGameState::Waiting;

    void StartCountDown() {
        countdownTimer = 3.f;
        state = EGameState::CountDown;
    }
    void StartRound() { state = EGameState::Playing; }
    void EndRound() { state = EGameState::Result; }

private:
    GameStateManager() = default;
};

class ParticleSystem {
public:
    static ParticleSystem& Get() {
        static ParticleSystem instance;
        return instance;
    }
    ParticleSystem(const ParticleSystem&) = delete;
    ParticleSystem& operator=(const ParticleSystem&) = delete;

    std::vector<GameObject*> pendingParticles;

    void Emit(glm::vec3 pos, glm::vec3 dir, int count) {
        for (int i = 0; i < count; i++) {
            glm::vec3 up = glm::abs(dir.y) < 0.9f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
            glm::vec3 tangent = glm::normalize(glm::cross(up, dir));
            glm::vec3 bitangent = glm::normalize(glm::cross(dir, tangent));

            float rx = ((float)rand() / RAND_MAX * 2.f - 1.f);
            float ry = ((float)rand() / RAND_MAX * 2.f - 1.f);
            float rz = ((float)rand() / RAND_MAX * 2.f - 1.f);
            glm::vec3 vel = glm::normalize(dir * (0.3f + rz * 0.7f) + // 메인 방향 성분
                                           tangent * rx * 0.8f +      // 옆 퍼짐
                                           bitangent * ry * 0.8f      // 위아래 퍼짐
                                           ) *
                            (4.f + (float)rand() / RAND_MAX * 4.f);
            auto* obj = new GameObject();
            obj->transform.position = pos;
            obj->transform.scale = glm::vec3(0.1f);

            Mesh* mesh = ResourceManager::Get().GetMesh("particle");
            Texture* tex = ResourceManager::Get().GetTexture("particle");
            obj->AddComponent(new MeshRenderer(mesh, new Material(tex)));
            obj->AddComponent(new ParticleComponent(-vel));

            pendingParticles.push_back(obj);
        }
    }

private:
    ParticleSystem() = default;
};


// RoundTimer
// 라운드 시간 관리. GameLoop이 소유.
//
//   duration  - 라운드 총 시간 (기본 60초)
//   remaining - 남은 시간
//   isRunning - 타이머 동작 중인지
//
// 함수:
//   Start(d)    - 라운드 시작. duration = remaining = d
//   Update(dt)  - 매 프레임 호출. remaining 감소
//   IsExpired() - remaining <= 0 이면 true. GameLoop에서 게임 종료 판단
//   Reset()     - 타이머 초기화
class RoundTimerComponent : public Component {
public:
    float duration  = 0.f;
    float remainTime = 0.f;
    bool  isRunning = false;

    void StartRound(float d) {
        duration = remainTime = d;
        isRunning = true;
    }

    virtual void Update(float dt) override{
        if (isRunning)
            remainTime -= dt;
    }
    bool IsExpired() const { return isRunning && remainTime <= 0.f;
    }
    void Reset() {
        remainTime = duration = 0.f;
        isRunning = false;
        //TODO : ScoreManager::Get().Reset()도 여기서 같이 해도 될듯?
        //TODO : world의 타겟들 다 지우는 거 
    }
};

// TargetSpawner
// 타겟 자동 생성. GameLoop이 소유.
//
// 멤버:
//   spawnInterval - 몇 초마다 생성 (기본 2초)
//   maxTargets    - 최대 동시 타겟 수 (기본 5개)
//   spawnTimer    - 누적 시간
//   spawnRange    - 생성 범위 (기본 ±5)
//   world*        - GameLoop의 world 포인터. 생성한 타겟 여기에 추가
// 
// 함수:
//   Update(dt)   - 매 프레임 호출. 타이머 누적 후 조건 맞으면 SpawnTarget()
//   SpawnTarget() - 랜덤 위치에 GameObject 생성 + TargetLogic 붙여서 world에 추가

class TargetSpawnerComponent : public Component {
public:
    float spawnInterval = 2.0f;
    int   maxTargets    = 10;
    float spawnTimer    = 0.f;
    int spawnAmount = 0;

    std::vector<GameObject*>* world3d = nullptr;
    std::vector<TargetObject*>* targetsToSpawn = nullptr;

    virtual void Update(float dt) override{
        if (!targetsToSpawn)
            return;
        if (GameStateManager::Get().state == EGameState::CountDown || GameStateManager::Get().state == EGameState::Result)
            return;

        spawnInterval = (float)rand() / RAND_MAX * 2.0f + 1.0f;
        spawnTimer += dt;
        if (spawnTimer < spawnInterval) return;
        spawnTimer = 0.f;

        int alive = 0;
        for (auto* obj : *world3d)
            if (obj->state == ETargetState::Spawned && dynamic_cast<TargetObject*>(obj))
                alive++;

        if (alive >= maxTargets) return;

        spawnAmount = rand() % 3 + 1;
        for (int i=0; i<spawnAmount; ++i) SpawnTarget();
    }

    void SpawnTarget() {
        float x = ((float)rand() / RAND_MAX * 2.f - 1.f) * 9.5f;
        float z = round(-(float)rand() / RAND_MAX * 10.f);

        TargetObject* target = new TargetObject();
        target->transform.position = glm::vec3(x, 0.0f, z);
        target->transform.scale = glm::vec3(0.2f, 0.2f, 0.05f);
        target->transform.rotation = glm::vec3(-90.f,0.f, 0.f);

        Mesh* mesh = ResourceManager::Get().GetMesh("target");
        Texture* tex = ResourceManager::Get().GetTexture("target");

        target->AddComponent(new MeshRenderer(mesh, new Material(tex)));
        target->AddComponent(new TargetController());
        target->onHitEmit = [](glm::vec3 pos, glm::vec3 dir) { ParticleSystem::Get().Emit(pos, dir, 20); };

        targetsToSpawn->push_back(target);
    }
};

class HUDComponent : public Component {
public:
    WeaponSystem* weaponSystem = nullptr;
    RoundTimerComponent* roundTimer = nullptr;
    std::vector<GameObject*>* world2d = nullptr;
    float digitSize = 48.f;

    std::vector<NumberController*> timeDigits;
    std::vector<NumberController*> scoreDigits;
    std::vector<NumberController*> accuracyDigits;
    std::vector<NumberController*> ammoDigits;

    NumberController* countdownDigit;
    NumberController* startSymbol;
    NumberController* restartSymbol;


    HUDComponent(std::vector<GameObject*>* world2d, WeaponSystem* ws, RoundTimerComponent* rt)
        : world2d(world2d), weaponSystem(ws), roundTimer(rt) {}

    NumberController* AddDigit(float ancX, float ancY, float offX, float offY) {
        auto& rm   = ResourceManager::Get();
        auto* mat  = new Material({
            rm.GetTexture("num0"), rm.GetTexture("num1"), rm.GetTexture("num2"),
            rm.GetTexture("num3"), rm.GetTexture("num4"), rm.GetTexture("num5"),
            rm.GetTexture("num6"), rm.GetTexture("num7"), rm.GetTexture("num8"),
            rm.GetTexture("num9")
        });
        auto* obj  = new GameObject();
        auto* ctrl = new NumberController(mat, ancX, ancY, offX, offY, digitSize, digitSize);
        obj->AddComponent(ctrl);
        obj->AddComponent(new OrthogonalRenderer(rm.GetMesh("crosshair"), mat));
        world2d->push_back(obj);
        return ctrl;
    }

    NumberController* AddSymbol(const std::string& texName, float ancX, float ancY, float offX, float offY) {
        auto& rm  = ResourceManager::Get();
        auto* mat = new Material(rm.GetTexture(texName));
        auto* obj = new GameObject();
        auto* ctrl = new NumberController(mat, ancX, ancY, offX, offY, digitSize * 0.5f, digitSize * 0.5f);
        obj->AddComponent(ctrl);
        obj->AddComponent(new OrthogonalRenderer(rm.GetMesh("crosshair"), mat));
        world2d->push_back(obj);
        return ctrl;
    }

    NumberController* AddSymbol(const std::string& texName, float ancX, float ancY, float offX, float offY, float w, float h) {
        auto& rm = ResourceManager::Get();
        auto* mat = new Material(rm.GetTexture(texName));
        auto* obj = new GameObject();
        auto* ctrl = new NumberController(mat, ancX, ancY, offX, offY, w, h);
        obj->AddComponent(ctrl);
        obj->AddComponent(new OrthogonalRenderer(rm.GetMesh("crosshair"), mat));
        world2d->push_back(obj);
        return ctrl;
    }

    void Update(float dt) override {

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


        if (GameStateManager::Get().state != EGameState::Playing)
            return;

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
};




class GameManagerComponent : public Component {
public:
    RoundTimerComponent* roundTimer = nullptr;
    WeaponSystem* weaponSystem = nullptr;
    std::vector<GameObject*>* world3d = nullptr;

    GameManagerComponent(RoundTimerComponent* rt, WeaponSystem* ws, std::vector<GameObject*>* w) 
        : roundTimer(rt), weaponSystem(ws), world3d(w) {}

    void Update(float dt) override { 

        if (InputManager::Get().IsKeyDown(GLFW_KEY_G) && !m_prevG)
            if (GameStateManager::Get().state == EGameState::Waiting ||
                GameStateManager::Get().state == EGameState::Result) {
                StartCountDown();
            }

        m_prevG = InputManager::Get().IsKeyDown(GLFW_KEY_G);

        if (InputManager::Get().IsKeyDown(GLFW_KEY_H) && !m_prevH) {
            EndRound();
        }

        m_prevH= InputManager::Get().IsKeyDown(GLFW_KEY_H);
        
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

private:
    void StartRound(){ 
        GameStateManager::Get().StartRound();
        ScoreManager::Get().Reset();
        if (roundTimer)
            roundTimer->StartRound(10.f);
        if (weaponSystem) {
            weaponSystem->StartRound();
        }
    }

    void EndRound() { 
        GameStateManager::Get().EndRound();
        if (roundTimer)
            roundTimer->Reset();
        for (auto* obj : *world3d) {
            if (dynamic_cast<TargetObject*>(obj)) {
                obj->state = ETargetState::Dying;
            }
        }
    }

    void StartCountDown() { 
        GameStateManager::Get().StartCountDown();
        for (auto* obj : *world3d) {
            if (dynamic_cast<TargetObject*>(obj)) {
                obj->state = ETargetState::Dying;
            }
        }
    }

    bool m_prevG = false;
    bool m_prevH = false;
};

#endif // AIMLAB_MANAGER_HPP