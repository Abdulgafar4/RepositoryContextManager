#include <gtest/gtest.h>
#include "git_info.hpp"
#include <filesystem>
#include <fstream>


class GitInfoTest: public ::testing::Test {
protected:
    std::filesystem::path testRepoPath;
    void SetUp() override {
        testRepoPath = std::filesystem::temp_directory_path() / "test_git_repo";
        std::filesystem::create_directories(testRepoPath);
    } 
    
    void TearDown() override {
        if(std::filesystem::exists(testRepoPath)) {
            std::filesystem::remove_all(testRepoPath);
        }
    }


    void createTestGitRepo() {
        std::filesystem::current_path(testRepoPath);
        system("git init");
        system("git config user.email test@gmail.com");
        system("git config user.name TestUser");

        std::ofstream file(testRepoPath / "test.txt");
        file << "test content";
        file.close();

        system("git add .");
        system("git commit -m Initial Commit");
    }
};

TEST_F(GitInfoTest, ValidGitRepository) {
    createTestGitRepo();

    auto info = gitInfo::getGitData(testRepoPath.string());

    EXPECT_TRUE(info.m_isGitRepository);
    EXPECT_FALSE(info.m_commit.empty());
    EXPECT_FALSE(info.m_branch.empty());
    EXPECT_FALSE(info.m_author.empty());
    EXPECT_FALSE(info.m_date.empty());
}

TEST_F(GitInfoTest, NonGitDirectory) {
    
    auto info = gitInfo::getGitData(testRepoPath.string());
    
    EXPECT_FALSE(info.m_isGitRepository);
    EXPECT_TRUE(info.m_commit.empty());
    EXPECT_TRUE(info.m_branch.empty());
    EXPECT_TRUE(info.m_author.empty());
    EXPECT_TRUE(info.m_date.empty());
}

TEST_F(GitInfoTest, NonExistentGitRepo) {
    std::filesystem::path fakePath = testRepoPath / "not_existed";
    auto info = gitInfo::getGitData(fakePath.string());
    EXPECT_FALSE(info.m_isGitRepository);
}

TEST_F(GitInfoTest, CorrectBranchName) {
    createTestGitRepo();

    auto info = gitInfo::getGitData(testRepoPath.string());

    EXPECT_TRUE(info.m_branch == "main" || info.m_branch == "master");
}

TEST_F(GitInfoTest, AuthorFormat) {
    createTestGitRepo();

    auto info = gitInfo::getGitData(testRepoPath.string());
    
    EXPECT_NE(info.m_author.find("TestUser"), std::string::npos);
    EXPECT_NE(info.m_author.find("test@gmail.com"), std::string::npos);
    EXPECT_NE(info.m_author.find("<"), std::string::npos);
    EXPECT_NE(info.m_author.find(">"), std::string::npos);

}
