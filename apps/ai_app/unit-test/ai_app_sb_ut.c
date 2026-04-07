/************************************************************************
 * ai_app subscription list unit tests
 ************************************************************************/

#include "ai_app_mission_ids.h"
#include "ai_app_mission_subscribe.h"

#include "utassert_stubs.h"

void UtTest_Sb_Run(void)
{
    const uint16_t *vals;
    size_t          n;

    n = AI_APP_Mission_GetSubscriptionSbValues(&vals);
    UtAssert_True(n == 2, "subscription count");
    UtAssert_True(vals != NULL, "subscription ptr");
    UtAssert_True(vals[0] == AI_APP_SB_VALUE_CFE_ES_HK_TLM, "ES HK value");
    UtAssert_True(vals[1] == AI_APP_SB_VALUE_SENSORS_HK_TLM, "Sensors HK value");
}
