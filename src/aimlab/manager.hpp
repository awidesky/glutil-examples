#ifndef AIMLAB_MANAGER_HPP
#define AIMLAB_MANAGER_HPP

#include "component.hpp"
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <GLFW/glfw3.h>
#include <algorithm>

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
class RoundTimer {
public:
    float duration  = 60.f;
    float remainTime = 60.f;
    bool  isRunning = false;

    void Start(float d) {
        duration = remainTime = d;
        isRunning = true;
    }
    void Update(float dt) {
        if (isRunning)
            remainTime -= dt;
    }
    bool IsExpired() const { return isRunning && remainTime <= 0.f;
    }
    void Reset() {
        remainTime = duration;
        isRunning = false;
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

class TargetSpawner {
public:
    float spawnInterval = 2.0f;
    int   maxTargets    = 5;
    float spawnTimer    = 0.f;
    float spawnRange    = 5.0f;

    std::vector<GameObject*>* world = nullptr;

    void Update(float dt) {
        if (!world) return;

        spawnTimer += dt;
        if (spawnTimer < spawnInterval) return;
        spawnTimer = 0.f;

        int alive = 0;
        for (auto* obj : *world)
            if (obj->active && obj->GetComponent<TargetLogic>())
                alive++;

        if (alive >= maxTargets) return;
        SpawnTarget();
    }

    void SpawnTarget() {
        float x = ((float)rand() / RAND_MAX * 2.f - 1.f) * spawnRange;
        float z = ((float)rand() / RAND_MAX * 2.f - 1.f) * spawnRange;

        GameObject* target = new GameObject();
        target->transform.position = glm::vec3(x, 1.f, z);
        target->transform.scale    = glm::vec3(1.f, 1.5f, 0.1f);

        // 나중에 추가할 것들 (mesh)

        target->AddComponent(new TargetLogic());
        world->push_back(target);
    }
};

#endif // AIMLAB_MANAGER_HPP