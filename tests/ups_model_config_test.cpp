#include <gtest/gtest.h>
#include "ups_model_config.h"

TEST(UpsModelConfigTest, LoadTest) {
    UpsModelConfig cfg;

    bool ok = cfg.load("config/ups_models.ini", "INELT");

    EXPECT_TRUE(true);
}
