/************************************************************************
 * ai_app SB subscription list (numeric SB values for CFE_SB_ValueToMsgId)
 ************************************************************************/

#include "ai_app_mission_subscribe.h"

#include "ai_app_mission_ids.h"

static const uint16_t AI_APP_SubscriptionSbValues[] = {
    AI_APP_SB_VALUE_CFE_ES_HK_TLM,
    AI_APP_SB_VALUE_SENSORS_HK_TLM,
};

size_t AI_APP_Mission_GetSubscriptionSbValues(const uint16_t **out_vals)
{
    if (out_vals != NULL)
    {
        *out_vals = AI_APP_SubscriptionSbValues;
    }
    return sizeof(AI_APP_SubscriptionSbValues) / sizeof(AI_APP_SubscriptionSbValues[0]);
}
