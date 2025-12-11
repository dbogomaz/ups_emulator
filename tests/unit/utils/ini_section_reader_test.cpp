#include "ini_section_reader.h"

#include <gtest/gtest.h>

using namespace utils;

class IniSectionReaderTest : public ::testing::Test {
protected:
    std::string dataDir = std::string(TEST_DATA_DIR) + "/ini_section_reader";

    // Полный путь к файлу тестовых данных
    std::string data(const std::string& name) const { return dataDir + "/" + name; }
};

// Проверка корректного чтения двух секций
TEST_F(IniSectionReaderTest, ValidTwoSections) {
    IniSectionReader reader(data("valid_two_sections.ini"));
    ASSERT_TRUE(reader.ok()) << reader.lastError();
    ASSERT_EQ(reader.sections().size(), 2u);
    EXPECT_EQ(reader.sections()[0], "APC");
    EXPECT_EQ(reader.sections()[1], "INELT");
}

// Ошибка при отсутствии секций в файле
TEST_F(IniSectionReaderTest, NoSections) {
    IniSectionReader reader(data("no_sections.ini"));
    ASSERT_FALSE(reader.ok());
    EXPECT_EQ(reader.lastError(), "No sections found in file: " + data("no_sections.ini"));
}

// Ошибка при обнаружении дублирующихся секций
TEST_F(IniSectionReaderTest, DuplicateSections) {
    IniSectionReader reader(data("duplicate_sections.ini"));
    ASSERT_FALSE(reader.ok());
    EXPECT_TRUE(reader.lastError().find("Duplicate section name") != std::string::npos);
}

// Ошибка при пустом имени секции ([])
TEST_F(IniSectionReaderTest, EmptySectionName) {
    IniSectionReader reader(data("empty_section_name.ini"));
    ASSERT_FALSE(reader.ok());
    EXPECT_TRUE(reader.lastError().find("Empty section name") != std::string::npos);
}

// Проверка секции с пробелами вокруг имени
TEST_F(IniSectionReaderTest, SectionWithSpaces) {
    IniSectionReader reader(data("section_with_spaces.ini"));
    ASSERT_TRUE(reader.ok());
    ASSERT_EQ(reader.sections().size(), 1u);
    EXPECT_EQ(reader.sections()[0], "MODEL_X");
}

// Проверка корректной обработки комментариев и пустых строк
TEST_F(IniSectionReaderTest, CommentsAndEmptyLines) {
    IniSectionReader reader(data("comments_and_empty_lines.ini"));
    ASSERT_TRUE(reader.ok());
    ASSERT_EQ(reader.sections().size(), 2u);
    EXPECT_EQ(reader.sections()[0], "APC");
    EXPECT_EQ(reader.sections()[1], "INELT");
}

// Ошибка при отсутствии файла
TEST_F(IniSectionReaderTest, FileNotFound) {
    IniSectionReader reader(data("no_such_file.ini"));
    ASSERT_FALSE(reader.ok());
    EXPECT_TRUE(reader.lastError().find("Cannot open file") != std::string::npos);
}
