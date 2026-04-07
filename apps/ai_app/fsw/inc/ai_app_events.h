/**
 * @file
 * ai_app event IDs
 */
#ifndef AI_APP_EVENTS_H
#define AI_APP_EVENTS_H

/*
 * Event IDs are app-scoped. Keep them stable for ground scripts.
 */
enum
{
    AI_APP_EVT_INIT_INF = 1,

    AI_APP_EVT_TBL_REGISTER_ERR = 10,
    AI_APP_EVT_TBL_LOAD_ERR     = 11,
    AI_APP_EVT_TBL_MANAGE_ERR   = 12,
    AI_APP_EVT_TBL_GET_ERR      = 13,
    AI_APP_EVT_TBL_RELEASE_ERR  = 14,

    AI_APP_EVT_TBL_VALIDATE_CRIT = 20,
    AI_APP_EVT_TBL_VALIDATE_INF  = 21,

    AI_APP_EVT_INFER_TIME_INF   = 30,
    AI_APP_EVT_INFER_FAIL_CRIT  = 31
};

#endif /* AI_APP_EVENTS_H */

