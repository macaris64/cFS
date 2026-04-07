/************************************************************************
 * ai_app autograd unit tests (goldens from scripts/golden/autograd_golden.py)
 ************************************************************************/

#include "ai_app_autograd.h"

#include "utassert_stubs.h"

void UtTest_Autograd_Run(void)
{
    AI_APP_Graph g;
    uint32_t     ia, ib, ic, id;
    uint32_t     ix, iy, iz, i4, im;
    uint32_t     ip, iq, iu, iv;
    uint32_t     it, ie;

    AI_APP_Graph_Init(&g);

    /* Case A: d = exp(a*b), a=2, b=3 */
    UtAssert_True(AI_APP_Value_Leaf(&g, 2.0, &ia) == 0, "A leaf a");
    UtAssert_True(AI_APP_Value_Leaf(&g, 3.0, &ib) == 0, "A leaf b");
    UtAssert_True(AI_APP_Value_Mul(&g, ia, ib, &ic) == 0, "A mul");
    UtAssert_True(AI_APP_Value_Exp(&g, ic, &id) == 0, "A exp");
    AI_APP_Backward(&g, id);
    UtAssert_DoubleCmpAbs(g.nodes[ia].grad, 1210.2863804782053, 1e-9, "A grad a");
    UtAssert_DoubleCmpAbs(g.nodes[ib].grad, 806.85758698547022, 1e-9, "A grad b");
    UtAssert_DoubleCmpAbs(g.nodes[ic].grad, 403.42879349273511, 1e-9, "A grad c");
    UtAssert_DoubleCmpAbs(g.nodes[id].grad, 1.0, 1e-12, "A grad d");

    /* Case B: w = (x+y)*4 */
    AI_APP_Graph_Init(&g);
    UtAssert_True(AI_APP_Value_Leaf(&g, 1.5, &ix) == 0, "B leaf x");
    UtAssert_True(AI_APP_Value_Leaf(&g, 2.5, &iy) == 0, "B leaf y");
    UtAssert_True(AI_APP_Value_Add(&g, ix, iy, &iz) == 0, "B add");
    UtAssert_True(AI_APP_Value_Leaf(&g, 4.0, &i4) == 0, "B leaf 4");
    UtAssert_True(AI_APP_Value_Mul(&g, iz, i4, &im) == 0, "B mul");
    AI_APP_Backward(&g, im);
    UtAssert_DoubleCmpAbs(g.nodes[ix].grad, 4.0, 1e-12, "B grad x");
    UtAssert_DoubleCmpAbs(g.nodes[iy].grad, 4.0, 1e-12, "B grad y");
    UtAssert_DoubleCmpAbs(g.nodes[iz].grad, 4.0, 1e-12, "B grad z");

    /* Case C: v = u+u, u = p*q */
    AI_APP_Graph_Init(&g);
    UtAssert_True(AI_APP_Value_Leaf(&g, 0.5, &ip) == 0, "C leaf p");
    UtAssert_True(AI_APP_Value_Leaf(&g, 1.25, &iq) == 0, "C leaf q");
    UtAssert_True(AI_APP_Value_Mul(&g, ip, iq, &iu) == 0, "C mul u");
    UtAssert_True(AI_APP_Value_Add(&g, iu, iu, &iv) == 0, "C add v");
    AI_APP_Backward(&g, iv);
    UtAssert_DoubleCmpAbs(g.nodes[ip].grad, 2.5, 1e-12, "C grad p");
    UtAssert_DoubleCmpAbs(g.nodes[iq].grad, 1.0, 1e-12, "C grad q");
    UtAssert_DoubleCmpAbs(g.nodes[iu].grad, 2.0, 1e-12, "C grad u");
    UtAssert_DoubleCmpAbs(g.nodes[iv].grad, 1.0, 1e-12, "C grad v");

    /* Case D: e = exp(t) */
    AI_APP_Graph_Init(&g);
    UtAssert_True(AI_APP_Value_Leaf(&g, 0.25, &it) == 0, "D leaf t");
    UtAssert_True(AI_APP_Value_Exp(&g, it, &ie) == 0, "D exp");
    AI_APP_Backward(&g, ie);
    UtAssert_DoubleCmpAbs(g.nodes[it].grad, 1.2840254166877414, 1e-12, "D grad t");
    UtAssert_DoubleCmpAbs(g.nodes[ie].grad, 1.0, 1e-12, "D grad e");

    /* Pool exhaustion */
    AI_APP_Graph_Init(&g);
    {
        uint32_t i;
        uint32_t tmp;
        for (i = 0; i < AI_APP_MAX_NODES; i++)
        {
            UtAssert_True(AI_APP_Value_Leaf(&g, 1.0, &tmp) == 0, "pool fill");
        }
        UtAssert_True(AI_APP_Value_Leaf(&g, 1.0, &tmp) != 0, "pool exhausted");
    }
}
