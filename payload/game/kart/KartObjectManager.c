#include "KartObjectManager.h"

#include "../system/RaceConfig.h"
#include "../system/SaveManager.h"

bool speedModIsEnabled;
f32 speedModFactor;
f32 speedModReverseFactor;

extern f32 minDriftSpeedFactor;
extern f32 boostAccelerations[6];
extern f32 ai_808cb550;

KartObjectManager *KartObjectManager_ct(KartObjectManager *this);

static void my_KartObjectManager_createInstance(void) {
    switch (s_raceConfig->raceScenario.gameMode) {
    case GAME_MODE_OFFLINE_VS:
        speedModIsEnabled = vsSpeedModIsEnabled;
        break;
    case GAME_MODE_TIME_ATTACK:
        speedModIsEnabled = SaveManager_getTaRuleClass(s_saveManager) == SP_TA_RULE_CLASS_200CC;
        break;
    default:
        speedModIsEnabled = false;
    }
    speedModFactor = speedModIsEnabled ? 1.5f : 1.0f;
    speedModReverseFactor = 1.0f / speedModFactor;

    minDriftSpeedFactor = 0.55f / speedModFactor;
    boostAccelerations[0] = 3.0f * speedModFactor;
    ai_808cb550 = 70.0f * speedModFactor;

    u32 courseId = s_raceConfig->raceScenario.courseId;
    const u32 *courseSha1 = SaveManager_getCourseSha1(s_saveManager, courseId);
    SpFooter_onRaceStart(courseSha1, speedModIsEnabled);

    s_kartObjectManager = new(sizeof(KartObjectManager));
    KartObjectManager_ct(s_kartObjectManager);
}
PATCH_B(KartObjectManager_createInstance, my_KartObjectManager_createInstance);