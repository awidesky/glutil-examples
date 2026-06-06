#ifndef AIMLAB_MANAGER_HPP
#define AIMLAB_MANAGER_HPP

#include "component.hpp"
#include "mesh.hpp"
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <config.hpp>
#include "score.hpp"


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
    float duration  = 60.f;
    float remainTime = 60.f;
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
        remainTime = duration;
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

    std::vector<TargetObject*>* targetsToSpawn = nullptr;

    virtual void Update(float dt) override{
        if (!targetsToSpawn)
            return;

        spawnTimer += dt;
        if (spawnTimer < spawnInterval) return;
        spawnTimer = 0.f;

        int alive = 0;
        for (auto* obj : *targetsToSpawn)
            if (obj->state == ETargetState::Spawned && dynamic_cast<TargetObject*>(obj))
                alive++;

        if (alive >= maxTargets) return;
        SpawnTarget();
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

        targetsToSpawn->push_back(target);
    }

    void GetSpawnLocation(int& x, int& z){ 
        int minZ = -10;
        int maxZ = -5;
        float maxX = 9.5f;
        float minX = -9.5f;


    }
};

class HUDComponent : public Component {
public:
    WeaponSystem* weaponSystem = nullptr;
    RoundTimerComponent* roundTimer = nullptr;
    GameLoop* gEngine = nullptr;
    float digitSize = 48.f;

    std::vector<NumberController*> timeDigits;
    std::vector<NumberController*> scoreDigits;
    std::vector<NumberController*> accuracyDigits;
    std::vector<NumberController*> ammoDigits;

    HUDComponent(GameLoop* loop, WeaponSystem* ws, RoundTimerComponent* rt)
        : gEngine(loop), weaponSystem(ws), roundTimer(rt) {}

    NumberController* AddDigit(float ancX, float ancY, float offX, float offY) {
        auto& rm   = ResourceManager::Get();
        auto* mat  = new Material({
            rm.GetTexture("num0"), rm.GetTexture("num1"), rm.GetTexture("num2"),
            rm.GetTexture("num3"), rm.GetTexture("num4"), rm.GetTexture("num5"),
            rm.GetTexture("num6"), rm.GetTexture("num7"), rm.GetTexture("num8"),
            rm.GetTexture("num9")
        });
        auto* obj  = new GameObject();
        auto* ctrl = new NumberController(mat, ancX, ancY, offX, offY, digitSize);
        obj->AddComponent(ctrl);
        obj->AddComponent(new OrthogonalRenderer(rm.GetMesh("crosshair"), mat));
        gEngine->world2d.push_back(obj);
        return ctrl;
    }

    void AddSymbol(const std::string& texName, float ancX, float ancY, float offX, float offY) {
        auto& rm  = ResourceManager::Get();
        auto* mat = new Material(rm.GetTexture(texName));
        auto* obj = new GameObject();
        obj->AddComponent(new NumberController(mat, ancX, ancY, offX, offY, digitSize * 0.5f));
        obj->AddComponent(new OrthogonalRenderer(rm.GetMesh("crosshair"), mat));
        gEngine->world2d.push_back(obj);
    }

    void Update(float dt) override {
        (void)dt;

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

        // 탄약
        if (weaponSystem && ammoDigits.size() >= 4) {
            int cur = weaponSystem->currentAmmo;
            int max = weaponSystem->maxAmmo;
            ammoDigits[0]->SetDigit(cur / 10);
            ammoDigits[1]->SetDigit(cur % 10);
            ammoDigits[2]->SetDigit(max / 10);
            ammoDigits[3]->SetDigit(max % 10);
        }
    }
};



#endif // AIMLAB_MANAGER_HPP