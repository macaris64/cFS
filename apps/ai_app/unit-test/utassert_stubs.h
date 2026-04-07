/************************************************************************
 * Minimal UtAssert-style macros for host builds without full cFE UT tree.
 * Compatible with nasa/cFE utassert naming for future bundle integration.
 ************************************************************************/
#ifndef UTASSERT_STUBS_H
#define UTASSERT_STUBS_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

extern int AI_APP_UT_Failures;

#define UtAssert_True(expr, Msg)                                                                                       \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(expr))                                                                                                   \
        {                                                                                                              \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, (Msg));                                                    \
            ++AI_APP_UT_Failures;                                                                                      \
        }                                                                                                              \
    } while (0)

#define UtAssert_DoubleCmpAbs(a, b, tol, Msg)                                                                          \
    do                                                                                                                 \
    {                                                                                                                  \
        if (fabs((double)(a) - (double)(b)) > (double)(tol))                                                          \
        {                                                                                                              \
            printf("FAIL %s:%d: %s (got %.17g expected %.17g tol %.17g)\n", __FILE__, __LINE__, (Msg), (double)(a),   \
                   (double)(b), (double)(tol));                                                                        \
            ++AI_APP_UT_Failures;                                                                                      \
        }                                                                                                              \
    } while (0)

#endif /* UTASSERT_STUBS_H */
