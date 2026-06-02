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

// ScoreManager
// 싱글톤. 게임 점수, 정확도 관리.
// 함수:
//   RecordHit(reactionTime) - 타겟 맞을 때 호출. score += 100 / reactionTime
//   RecordMiss()            - 빗나갈 때 호출. totalShots만 증가
//   GetAccuracy()           - 정확도 반환. hits / totalShots * 100
//   Reset()                 - 라운드 시작할 때 초기화

class ScoreManager {
public:
    static ScoreManager& Get() {
        static ScoreManager scoremanager;
        return scoremanager;
    }
    ScoreManager(const ScoreManager&) = delete;
    ScoreManager& operator=(const ScoreManager&) = delete;

    int   score      = 0;
    int   totalShot  = 0;
    int   hit        = 0;

    void RecordHit(float reactionTime) {
        hit++;
        totalShot++;
        score += (int)(100.f / reactionTime);
    }
    void RecordMiss() { totalShot++; }

    float GetAccuracy() const {
        if (totalShot == 0) return 0.f;
        return (float)hit / (float)totalShot * 100.f;
    }

    void Reset() { score = totalShot = hit = 0; }

private:
    ScoreManager() = default;
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
    float spawnRange    = 10.0f;

    std::vector<GameObject*>* world = nullptr;

    virtual void Update(float dt) override{
        if (!world) return;

        spawnTimer += dt;
        if (spawnTimer < spawnInterval) return;
        spawnTimer = 0.f;

        int alive = 0;
        for (auto* obj : *world)
            if (obj->active && dynamic_cast<TargetObject*>(obj))
                alive++;

        if (alive >= maxTargets) return;
        SpawnTarget();
    }

    void SpawnTarget() {
        float x = ((float)rand() / RAND_MAX * 2.f - 1.f) * spawnRange;
        float y = ((float)rand() / RAND_MAX * 2.f - 1.f) * spawnRange;

        TargetObject* target = new TargetObject();
        target->transform.position = glm::vec3(x, 1.0f, y);
        target->transform.scale = glm::vec3(0.2f, 0.2f, 0.05f);

        Mesh* mesh = ResourceManager::Get().GetMesh("target");
        Texture* tex = ResourceManager::Get().GetTexture("target");

        target->AddComponent(new MeshRenderer(mesh, new Material(tex)));

        world->push_back(target);
    }
};

#endif // AIMLAB_MANAGER_HPP