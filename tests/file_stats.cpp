#include <gtest/gtest.h>
#include "renderer.hpp"
#include "cli.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>

class RendererTest : public ::testing::Test {
protected:
	void SetUp() override {
		testDir = std::filesystem::temp_directory_path() / "renderer_test";
		std::filesystem::create_directories(testDir);
	}

	void TearDown() override {
		if (std::filesystem::exists(testDir)) {
			std::filesystem::remove_all(testDir);
		}
	}
	void createTestFile(const std::filesystem::path& filepath, int numLines) {
		std::ofstream file(filepath);
		for (int i = 0; i < numLines; i++) {
			file << "Line: " << i << '\n';
		}
	}

	 size_t countOccurrences(const std::string& str, const std::string& substr) {
		 size_t count = 0;
		 size_t pos = 0;
		 while ((pos = str.find(substr, pos)) != std::string::npos) {
			 ++count;
			 pos += substr.length();
		 }
		 return count;
	}

	std::filesystem::path testDir;
};


TEST_F(RendererTest, SingleFileStatistics) {
	auto testFile = testDir / "test.txt";
	createTestFile(testFile, 10);

	cli::Options options;

	std::ostringstream output;
	output::writeFileStatistics(output, testFile, options);

	std::string result = output.str();
	EXPECT_NE(result.find("### Statistics"), std::string::npos);
	EXPECT_NE(result.find("Total Files: 1"), std::string::npos);
	EXPECT_NE(result.find("Total Lines: 10"), std::string::npos);
}

TEST_F(RendererTest, EmptyFileStatistics) {
	auto testFile = testDir / "empty.txt";
	createTestFile(testFile, 0);
	
	cli::Options options;
	std::ostringstream output;

	output::writeFileStatistics(output, testFile, options);
	std::string result = output.str();

	EXPECT_NE(result.find("Total Files: 1"), std::string::npos);
	EXPECT_NE(result.find("Total Lines: 0"), std::string::npos);

	size_t totalCount = countOccurrences(result, "Total");
	EXPECT_EQ(totalCount, 2);
}

TEST_F(RendererTest, MultipleFilePositiveStats) {
	createTestFile(testDir / "file1.txt", 1000);
	createTestFile(testDir / "file2.txt", 2000);
	createTestFile(testDir / "file3.txt", 3000);
	cli::Options options;
	std::ostringstream output;
	output::writeFileStatistics(output, testDir, options);
	std::string result = output.str();

	EXPECT_NE(result.find("Total Files: 3"), std::string::npos);
	EXPECT_NE(result.find("Total Lines: 6000"), std::string::npos);
	EXPECT_GT(result.length(), 30);    
	EXPECT_LT(result.length(), 200);   
}

TEST_F(RendererTest, CompleteValidation) {
	createTestFile(testDir / "test1.txt", 15);
	createTestFile(testDir / "test2.txt", 20);
	cli::Options options;
	std::ostringstream output;

	output::writeFileStatistics(output, testDir, options);
	std::string result = output.str();

	EXPECT_FALSE(result.empty());
	EXPECT_TRUE(result.length() > 30);
	EXPECT_LE(result.length(), 50);
}

TEST_F(RendererTest, FilteredResultsFewerFiles) {
	createTestFile(testDir / "file1.txt", 10);
	createTestFile(testDir / "file2.cpp", 20);
	createTestFile(testDir / "file3.cpp", 30);

	cli::Options optionsNoFilter;
	std::ostringstream outputNoFilter;

	output::writeFileStatistics(outputNoFilter, testDir, optionsNoFilter);
	std::string resultsNoFilter = outputNoFilter.str();

	cli::Options optionsWithFilter;
	optionsWithFilter.excludePattern = "*.cpp";
	optionsWithFilter.onExcludeFilter = [](const std::filesystem::path& p) {
		return p.extension() == ".cpp";
		};

	std::ostringstream outputWithFilter;
	output::writeFileStatistics(outputWithFilter, testDir, optionsWithFilter);
	std::string resultsWithFilter = outputWithFilter.str();

	EXPECT_NE(resultsNoFilter.find("Total Files: 3"), std::string::npos);
	EXPECT_NE(resultsWithFilter.find("Total Files: 1"), std::string::npos);
}

