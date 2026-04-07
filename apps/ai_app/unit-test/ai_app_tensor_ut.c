/************************************************************************
 * ai_app tensor unit tests (goldens from scripts/tensor_golden.py)
 ************************************************************************/

#include "ai_app_tensor.h"
#include "ai_app_telemetry.h"

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "utassert_stubs.h"

void UtTest_Tensor_Run(void)
{
    double x[4]    = {1.0, 0.0, -2.0, 0.5};
    double out[4];
    double lg[4]   = {1.0, 2.0, -0.5, 0.25};
    double pr[4];
    double sum;
    size_t i;

    const double w[6] = {0.1, 0.2, 0.3, -0.1, -0.2, 0.4};
    double         xv[2] = {2.0, -1.0};
    double         y[3];

    AI_APP_RmsNorm(x, out, 4);
    UtAssert_DoubleCmpAbs(out[0], 0.87286823573797656, 1e-12, "rmsnorm[0]");
    UtAssert_DoubleCmpAbs(out[1], 0.0, 1e-15, "rmsnorm[1]");
    UtAssert_DoubleCmpAbs(out[2], -1.7457364714759531, 1e-12, "rmsnorm[2]");
    UtAssert_DoubleCmpAbs(out[3], 0.43643411786898828, 1e-12, "rmsnorm[3]");

    AI_APP_Softmax(lg, pr, 4);
    UtAssert_DoubleCmpAbs(pr[0], 0.22656324748329623, 1e-12, "softmax[0]");
    UtAssert_DoubleCmpAbs(pr[1], 0.61586275863051365, 1e-12, "softmax[1]");
    UtAssert_DoubleCmpAbs(pr[2], 0.050553093694696231, 1e-12, "softmax[2]");
    UtAssert_DoubleCmpAbs(pr[3], 0.10702090019149402, 1e-12, "softmax[3]");
    sum = 0.0;
    for (i = 0; i < 4; i++)
    {
        sum += pr[i];
    }
    UtAssert_DoubleCmpAbs(sum, 1.0, 1e-14, "softmax sum");

    AI_APP_Linear(w, xv, y, 3, 2);
    UtAssert_DoubleCmpAbs(y[0], 0.0, 1e-12, "linear[0]");
    UtAssert_DoubleCmpAbs(y[1], 0.69999999999999996, 1e-12, "linear[1]");
    UtAssert_DoubleCmpAbs(y[2], -0.80000000000000004, 1e-12, "linear[2]");

    {
        uint8_t pl[4] = {255, 128, 0, 0};
        double  f[32];
        size_t  k;
        AI_APP_NormalizePayloadToFeatures(pl, sizeof(pl), f, AI_APP_TLM_FEATURE_DIM);
        UtAssert_DoubleCmpAbs(f[0], 1.0, 1e-12, "tlm norm[0]");
        UtAssert_DoubleCmpAbs(f[1], 128.0 / 255.0, 1e-12, "tlm norm[1]");
        UtAssert_DoubleCmpAbs(f[2], 0.0, 1e-15, "tlm norm[2]");
        UtAssert_DoubleCmpAbs(f[3], 0.0, 1e-15, "tlm norm[3]");
        for (k = 4; k < AI_APP_TLM_FEATURE_DIM; k++)
        {
            UtAssert_DoubleCmpAbs(f[k], 0.0, 1e-15, "tlm zero pad");
        }
    }
}
