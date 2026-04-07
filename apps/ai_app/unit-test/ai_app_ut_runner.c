/************************************************************************
 * ai_app host unit test runner (no cFE required)
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>

extern int AI_APP_UT_Failures;

extern void UtTest_Autograd_Run(void);
extern void UtTest_Tensor_Run(void);
extern void UtTest_Gpt_Run(void);
extern void UtTest_Sb_Run(void);
extern void UtTest_Tbl_Run(void);

int main(void)
{
    printf("ai_app UT: autograd\n");
    UtTest_Autograd_Run();
    printf("ai_app UT: tensor\n");
    UtTest_Tensor_Run();
    printf("ai_app UT: gpt\n");
    UtTest_Gpt_Run();
    printf("ai_app UT: sb\n");
    UtTest_Sb_Run();
    printf("ai_app UT: tbl\n");
    UtTest_Tbl_Run();

    if (AI_APP_UT_Failures != 0)
    {
        printf("ai_app UT: %d failure(s)\n", AI_APP_UT_Failures);
        return EXIT_FAILURE;
    }
    printf("ai_app UT: all tests passed\n");
    return EXIT_SUCCESS;
}
