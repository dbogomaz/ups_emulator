#include "fs_utils.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <string>

using namespace utils;

class FsUtilsTest : public ::testing::Test {
protected:
    std::string binDir;

    void SetUp() override {
        binDir = getBinaryDir();
        // Общие проверки, которые нужны почти во всех тестах
        ASSERT_FALSE(binDir.empty());
        ASSERT_TRUE(std::filesystem::exists(binDir));
        ASSERT_TRUE(std::filesystem::is_directory(binDir));
    }
};

// Абсолютный путь возвращается без изменений
TEST_F(FsUtilsTest, ResolvePath_Absolute_ReturnsSame) {
    std::string input = "/etc/passwd";
    std::string output = resolvePath(input);
    EXPECT_EQ(output, input);
}

// Относительный путь дополняется директорией бинарника
TEST_F(FsUtilsTest, ResolvePath_Relative_UsesBinaryDir) {
    std::string rel = "config/ups.ini";
    std::string full = resolvePath(rel);
    // full должен начинаться с binDir + "/"
    std::string expectedPrefix = binDir + "/";
    EXPECT_EQ(full.substr(0, expectedPrefix.size()), expectedPrefix);
    // И дальше — относительный путь
    EXPECT_EQ(full.substr(expectedPrefix.size()), rel);
}

// Путь не должен быть пустым
TEST_F(FsUtilsTest, GetBinaryDir_NotEmpty) { EXPECT_FALSE(binDir.empty()); }

// Если передать ".", должна вернуться "<bindir>/."
TEST_F(FsUtilsTest, ResolvePath_CurrentDirFallback) {
    std::string out = resolvePath(".");
    std::string expected = binDir + "/.";
    EXPECT_EQ(out, expected);
}
