#ifndef AIMLAB_SCORE_HPP
#define AIMLAB_SCORE_HPP
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

    int score = 0;
    int totalShot = 0;
    int hit = 0;

    void RecordHit(float reactionTime) {
        hit++;
        totalShot++;
        score += (int)(100.f / reactionTime);
    }
    void RecordMiss() { totalShot++; }

    float GetAccuracy() const {
        if (totalShot == 0)
            return 0.f;
        return (float)hit / (float)totalShot * 100.f;
    }

    void Reset() { score = totalShot = hit = 0; }

private:
    ScoreManager() = default;
};

#endif // AIMLAB_SCORE_HPP