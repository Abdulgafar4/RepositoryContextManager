#include <gtest/gtest.h>
#include "filter.hpp"
#include <filesystem>
#include <fstream>
#include <string>

struct TestCase {
    std::string fileName;
    std::string includePattern;
    std::string excludePattern;
    bool expectedResult;
};

class FilterManagerParamTest : public ::testing::TestWithParam<TestCase> {

protected:
    void SetUp() override {
        std::ofstream("test.txt") << "content";
        std::ofstream("test.cpp") << "content";
        std::ofstream("test.hpp") << "content";
    }

    void TearDown() override {
        std::filesystem::remove("test.txt");
        std::filesystem::remove("test.cpp");
        std::filesystem::remove("test.hpp");
    }
  
};

TEST_P(FilterManagerParamTest, TestMultipleFiles) {
    TestCase testCase = GetParam();
    cli::Options options;
    options.includePattern = testCase.includePattern;
    options.excludePattern = testCase.excludePattern;

    //SetUp filters

    options.onIncludeFilter = [testCase](const std::filesystem::path& p) {
            if (testCase.includePattern.empty()) return true;
            if (testCase.includePattern == "*.txt") return p.extension() == ".txt";
            if (testCase.includePattern == "*.cpp") return p.extension() == ".cpp";
            return true;
        };
    
    options.onExcludeFilter = [testCase](const std::filesystem::path& p) {
        if (testCase.excludePattern.empty()) return false;
        if (testCase.excludePattern == "*.txt") return p.extension() == ".txt";
        if (testCase.excludePattern == "*.cpp") return p.extension() == ".cpp";
        return false;
        };

    Filter::FilterManager manager(options);
    bool result = manager.isMatchingFilters(testCase.fileName);

    EXPECT_EQ(result, testCase.expectedResult);

}

// Test Data

INSTANTIATE_TEST_SUITE_P(
    MultipleScenarios,
    FilterManagerParamTest,
    ::testing::Values(
        TestCase{ "test.txt","*.txt","",true },
        TestCase{ "test.cpp", "*.txt","",false },
        TestCase{ "test.txt","","*.txt",false },
        TestCase{ "test.cpp","","*.txt",true }
    )
);