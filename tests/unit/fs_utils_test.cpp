#include <gtest/gtest.h>
#include "fs_utils.h"
#include <string>
#include <filesystem>

using namespace utils;

TEST(FsUtilsTest, ResolvePath_Absolute_ReturnsSame)
{
    std::string input = "/etc/passwd";
    std::string output = resolvePath(input);
    EXPECT_EQ(output, input);
}

TEST(FsUtilsTest, ResolvePath_Relative_UsesBinaryDir)
{
    std::string rel = "config/ups.ini";
    std::string full = resolvePath(rel);

    // Проверяем: путь ДОЛЖЕН начинаться с директории бинарника
    std::string binDir = getBinaryDir();

    ASSERT_FALSE(binDir.empty());
    ASSERT_NE(binDir, ".");

    // full должен начинаться с binDir + "/"
    std::string expectedPrefix = binDir + "/";
    EXPECT_EQ(full.substr(0, expectedPrefix.size()), expectedPrefix);

    // И дальше — относительный путь
    EXPECT_EQ(full.substr(expectedPrefix.size()), rel);
}

TEST(FsUtilsTest, GetBinaryDir_NotEmpty)
{
    std::string binDir = getBinaryDir();

    // Не должен быть пустым
    EXPECT_FALSE(binDir.empty());

    // Должен быть существующий каталог
    ASSERT_TRUE(std::filesystem::exists(binDir));
    ASSERT_TRUE(std::filesystem::is_directory(binDir));
}

TEST(FsUtilsTest, ResolvePath_CurrentDirFallback)
{
    // Если передать ".", должна вернуться "<bindir>/."
    std::string out = resolvePath(".");

    std::string binDir = getBinaryDir();
    ASSERT_FALSE(binDir.empty());

    std::string expected = binDir + "/.";

    EXPECT_EQ(out, expected);
}
