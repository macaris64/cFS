/************************************************************************
 * ai_app telemetry feature extraction
 ************************************************************************/

#include "ai_app_telemetry.h"

#include <string.h>

void AI_APP_NormalizePayloadToFeatures(const uint8_t *payload, size_t payload_len, double *features, size_t feature_dim)
{
    size_t i;
    if (features == NULL || feature_dim == 0)
    {
        return;
    }
    memset(features, 0, feature_dim * sizeof(double));
    if (payload == NULL || payload_len == 0)
    {
        return;
    }
    for (i = 0; i < feature_dim && i < payload_len; i++)
    {
        features[i] = (double)payload[i] / 255.0;
    }
}
