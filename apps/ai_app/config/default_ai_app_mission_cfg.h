/************************************************************************
 * ai_app mission configuration (defaults — microgpt toy dimensions)
 ************************************************************************/
#ifndef DEFAULT_AI_APP_MISSION_CFG_H
#define DEFAULT_AI_APP_MISSION_CFG_H

/* Match scripts/reference/microgpt.py small GPT */
#define AI_APP_GPT_VOCAB_SIZE 17u
#define AI_APP_GPT_N_EMBD 16u
#define AI_APP_GPT_BLOCK_SIZE 16u
#define AI_APP_GPT_N_HEAD 4u
#define AI_APP_GPT_N_LAYER 1u

/* Upper bound for static arrays (n_layer must be <= this) */
#define AI_APP_GPT_MAX_LAYER 4u

#define AI_APP_GPT_MLP_FACTOR 4u
#define AI_APP_GPT_MLP_HIDDEN ((AI_APP_GPT_N_EMBD) * (AI_APP_GPT_MLP_FACTOR))

#define AI_APP_TLM_FEATURE_DIM (AI_APP_GPT_N_EMBD)

#endif /* DEFAULT_AI_APP_MISSION_CFG_H */
