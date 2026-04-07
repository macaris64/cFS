/************************************************************************
 * ai_app autograd — no heap; topological backward
 ************************************************************************/

#include "ai_app_autograd.h"

#include <math.h>
#include <string.h>

void AI_APP_Graph_Init(AI_APP_Graph *g)
{
    if (g == NULL)
    {
        return;
    }
    memset(g, 0, sizeof(*g));
    {
        size_t i;
        for (i = 0; i < AI_APP_MAX_NODES; i++)
        {
            g->nodes[i].child0 = AI_APP_NODE_INVALID;
            g->nodes[i].child1 = AI_APP_NODE_INVALID;
        }
    }
}

static int alloc_node(AI_APP_Graph *g, uint32_t *out_idx)
{
    if (g == NULL || out_idx == NULL)
    {
        return -1;
    }
    if (g->count >= AI_APP_MAX_NODES)
    {
        return -1;
    }
    *out_idx = g->count;
    ++g->count;
    return 0;
}

int AI_APP_Value_Leaf(AI_APP_Graph *g, double data, uint32_t *out_idx)
{
    uint32_t idx;
    if (alloc_node(g, &idx) != 0)
    {
        return -1;
    }
    g->nodes[idx].data       = data;
    g->nodes[idx].grad       = 0.0;
    g->nodes[idx].op         = AI_APP_OP_LEAF;
    g->nodes[idx].child0     = AI_APP_NODE_INVALID;
    g->nodes[idx].child1     = AI_APP_NODE_INVALID;
    *out_idx                 = idx;
    return 0;
}

int AI_APP_Value_Add(AI_APP_Graph *g, uint32_t a, uint32_t b, uint32_t *out_idx)
{
    uint32_t idx;
    if (g == NULL || out_idx == NULL || a >= g->count || b >= g->count)
    {
        return -1;
    }
    if (alloc_node(g, &idx) != 0)
    {
        return -1;
    }
    g->nodes[idx].data   = g->nodes[a].data + g->nodes[b].data;
    g->nodes[idx].grad   = 0.0;
    g->nodes[idx].op     = AI_APP_OP_ADD;
    g->nodes[idx].child0 = a;
    g->nodes[idx].child1 = b;
    *out_idx             = idx;
    return 0;
}

int AI_APP_Value_Mul(AI_APP_Graph *g, uint32_t a, uint32_t b, uint32_t *out_idx)
{
    uint32_t idx;
    if (g == NULL || out_idx == NULL || a >= g->count || b >= g->count)
    {
        return -1;
    }
    if (alloc_node(g, &idx) != 0)
    {
        return -1;
    }
    g->nodes[idx].data   = g->nodes[a].data * g->nodes[b].data;
    g->nodes[idx].grad   = 0.0;
    g->nodes[idx].op     = AI_APP_OP_MUL;
    g->nodes[idx].child0 = a;
    g->nodes[idx].child1 = b;
    *out_idx             = idx;
    return 0;
}

int AI_APP_Value_Exp(AI_APP_Graph *g, uint32_t x, uint32_t *out_idx)
{
    uint32_t idx;
    if (g == NULL || out_idx == NULL || x >= g->count)
    {
        return -1;
    }
    if (alloc_node(g, &idx) != 0)
    {
        return -1;
    }
    g->nodes[idx].data   = exp(g->nodes[x].data);
    g->nodes[idx].grad   = 0.0;
    g->nodes[idx].op     = AI_APP_OP_EXP;
    g->nodes[idx].child0 = x;
    g->nodes[idx].child1 = AI_APP_NODE_INVALID;
    *out_idx             = idx;
    return 0;
}

typedef struct {
    uint8_t visited[(AI_APP_MAX_NODES + 7u) / 8u];
} AI_APP_VisitSet;

static void visit_clear(AI_APP_VisitSet *vs)
{
    memset(vs->visited, 0, sizeof(vs->visited));
}

static int visit_get(const AI_APP_VisitSet *vs, uint32_t i)
{
    size_t byte = (size_t)(i / 8u);
    int    bit  = (int)(i % 8u);
    return (vs->visited[byte] >> bit) & 1;
}

static void visit_set(AI_APP_VisitSet *vs, uint32_t i)
{
    size_t byte = (size_t)(i / 8u);
    int    bit  = (int)(i % 8u);
    vs->visited[byte] |= (uint8_t)(1u << bit);
}

static void build_topo_recursive(const AI_APP_Graph *g, uint32_t v, AI_APP_VisitSet *vs, uint32_t *order,
                                   size_t *order_len)
{
    if (v >= g->count || visit_get(vs, v))
    {
        return;
    }
    visit_set(vs, v);

    if (g->nodes[v].child0 != AI_APP_NODE_INVALID)
    {
        build_topo_recursive(g, g->nodes[v].child0, vs, order, order_len);
    }
    if (g->nodes[v].child1 != AI_APP_NODE_INVALID)
    {
        build_topo_recursive(g, g->nodes[v].child1, vs, order, order_len);
    }

    if (*order_len >= AI_APP_MAX_NODES)
    {
        return;
    }
    order[*order_len] = v;
    ++(*order_len);
}

int AI_APP_TopoSort(const AI_APP_Graph *g, uint32_t root_idx, uint32_t *order, size_t *order_len)
{
    AI_APP_VisitSet vs;
    if (g == NULL || order == NULL || order_len == NULL || root_idx >= g->count)
    {
        return -1;
    }
    *order_len = 0;
    visit_clear(&vs);
    build_topo_recursive(g, root_idx, &vs, order, order_len);
    return 0;
}

void AI_APP_Backward(AI_APP_Graph *g, uint32_t root_idx)
{
    uint32_t order[AI_APP_MAX_NODES];
    size_t   n;
    size_t   i;
    if (g == NULL || root_idx >= g->count)
    {
        return;
    }

    for (i = 0; i < g->count; i++)
    {
        g->nodes[i].grad = 0.0;
    }

    if (AI_APP_TopoSort(g, root_idx, order, &n) != 0)
    {
        return;
    }

    g->nodes[root_idx].grad = 1.0;

    for (i = n; i > 0; i--)
    {
        uint32_t     vidx = order[i - 1];
        AI_APP_ValueNode *v = &g->nodes[vidx];
        double       gv     = v->grad;

        switch (v->op)
        {
            case AI_APP_OP_LEAF:
                break;
            case AI_APP_OP_ADD:
                g->nodes[v->child0].grad += gv;
                g->nodes[v->child1].grad += gv;
                break;
            case AI_APP_OP_MUL:
                g->nodes[v->child0].grad += g->nodes[v->child1].data * gv;
                g->nodes[v->child1].grad += g->nodes[v->child0].data * gv;
                break;
            case AI_APP_OP_EXP:
                g->nodes[v->child0].grad += v->data * gv;
                break;
            default:
                break;
        }
    }
}
