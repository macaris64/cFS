/************************************************************************
 * ai_app — static-pool autograd (microgpt Value port)
 ************************************************************************/
#ifndef AI_APP_AUTOGRAD_H
#define AI_APP_AUTOGRAD_H

#include <stddef.h>
#include <stdint.h>

#ifndef AI_APP_MAX_NODES
#define AI_APP_MAX_NODES 8192u
#endif

#define AI_APP_NODE_INVALID 0xFFFFFFFFu

typedef enum {
    AI_APP_OP_LEAF = 0,
    AI_APP_OP_ADD,
    AI_APP_OP_MUL,
    AI_APP_OP_EXP
} AI_APP_OpKind;

typedef struct {
    double       data;
    double       grad;
    AI_APP_OpKind op;
    uint32_t     child0;
    uint32_t     child1;
} AI_APP_ValueNode;

typedef struct {
    AI_APP_ValueNode nodes[AI_APP_MAX_NODES];
    uint32_t         count;
} AI_APP_Graph;

void AI_APP_Graph_Init(AI_APP_Graph *g);

int AI_APP_Value_Leaf(AI_APP_Graph *g, double data, uint32_t *out_idx);

int AI_APP_Value_Add(AI_APP_Graph *g, uint32_t a, uint32_t b, uint32_t *out_idx);

int AI_APP_Value_Mul(AI_APP_Graph *g, uint32_t a, uint32_t b, uint32_t *out_idx);

int AI_APP_Value_Exp(AI_APP_Graph *g, uint32_t x, uint32_t *out_idx);

int AI_APP_TopoSort(const AI_APP_Graph *g, uint32_t root_idx, uint32_t *order, size_t *order_len);

void AI_APP_Backward(AI_APP_Graph *g, uint32_t root_idx);

#endif /* AI_APP_AUTOGRAD_H */
