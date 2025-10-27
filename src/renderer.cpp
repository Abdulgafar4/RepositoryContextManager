#include "renderer.hpp"

namespace output {


	void writeTokenCountTree(std::ostream& o,const std::filesystem::path& path, const cli::Options& opt) {
		o << "Token Count Tree:\n";
		if (opt.tokenCountThreshold > 0) {
		  o << "Showing files with " << opt.tokenCountThreshold << "+ tokens:\n";
		}
		Filter::FilterManager filter(opt);

		std::map<std::filesystem::path, std::size_t> fileTokens;
		for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
			if (entry.is_regular_file() && filter.isMatchingFilters(entry.path())) {
				auto relative = std::filesystem::relative(entry.path(), path);
				std::size_t tokens = countTokens(entry.path());

				if (tokens >= opt.tokenCountThreshold) {
					fileTokens[relative] = tokens;
				}
			}
		}

		displayTokenTree(fileTokens,o);
	}

	//if filename is empty target the console otherwise write to the file
	std::ostream& targetOut(std::ofstream& file, const std::string& filename) {
		if (filename.empty())
			return std::cout;
		file.open(filename);
		if (!file)
			throw std::runtime_error("Error opening output File: " + filename);
		return file;

	}


	void writeGitInfo(std::ostream& o, const std::filesystem::path& absolute) {
		o << "### GIT INFO \n\n";

		const auto repo = findGitRepository(absolute);
		gitInfo::GitInfo gi;
		if (!repo.empty()) gi = gitInfo::getGitData(repo.string());
		if (!gi.m_isGitRepository) {
			o << "Not a git repository\n\n";
		}
		else {
			o << "- Commit: " << gi.m_commit << "\n"
				<< "- Branch: " << gi.m_branch << "\n"
				<< "- Author: " << gi.m_author << "\n"
				<< "- Date: " << gi.m_date << "\n\n";
		}

	}

	bool writeCliCommands( const cli::Options& opt) {
			if (opt.showHelp) {
				showHelp();
				return true;
			}

			if (opt.showVersion) {
				showVersion();
				return true;
			}

			
			return false;

	}


	//Capturing cout output original_out is the pointer to the ofstream object  it saves the current cout destination
	//  then caughtOut is created for accepting the temporary  string buffer
	//later we redirect the cout to the current cout destination
	// rdbuf is useful for redirecting the code without changing the code 
	void writeFileStructure(std::ostream& o, const std::filesystem::path& path) {
		o << "Structure\n\n";
		std::streambuf* original_out = std::cout.rdbuf();
		std::ostringstream caughtOut;
		std::cout.rdbuf(caughtOut.rdbuf());

		//calling my function
		fsTravel::travelDirTree(path, 0);

		//restoring cout
		std::cout.rdbuf(original_out);
		o << caughtOut.str() << "\n\n";


	}

	void writeFileStatistics(std::ostream& o, const std::filesystem::path& path, const cli::Options& opt) {
		Filter::FilterManager filter(opt);
		std::size_t totalLines = 0;
		std::size_t totalFiles = 0;

		std::error_code err;

		if (std::filesystem::is_regular_file(path) && filter.isMatchingFilters(path)) {
			totalFiles += 1;
			totalLines += countLines(path);
		}

		else if (std::filesystem::is_directory(path)) {
			for (const auto& entry : std::filesystem::recursive_directory_iterator(path, err)) {
		
				if (err) continue;
				if (entry.is_regular_file() && filter.isMatchingFilters(entry.path())) {
					totalFiles += 1;
					totalLines += countLines(entry.path());
				}
			}
		}

		o << "### Statistics\n";
		o << "Total Files: " << totalFiles << '\n';
		o << "Total Lines: " << totalLines << '\n';

	}
	

	void writeFileContents(std::ostream& o, const std::filesystem::path& path,const cli::Options& opt) {
		//capturing the output

		std::streambuf* original_out = std::cout.rdbuf();
		std::ostringstream caughtOut;
		std::cout.rdbuf(caughtOut.rdbuf());
		
		Filter::FilterManager filter(opt);

		std::error_code err;

		if (std::filesystem::is_regular_file(path) && filter.isMatchingFilters(path)) {
			fsTravel::travelFileContents(path);
		}
		else if (std::filesystem::is_directory(path)) {
			for (const auto& entry : std::filesystem::recursive_directory_iterator(path, err)) {
				if (err) continue;
				if (entry.is_regular_file() && filter.isMatchingFilters(entry.path())) {
					readDisplayFile(entry.path());
				}
			}
		}

		std::cout.rdbuf(original_out);
		o << caughtOut.str();
		o << '\n';
	}


	void renderRepositoryContext(const std::string& filename, const cli::Options& opt) {
		try {

			if (opt.showTokenCountTree) {
				std::ofstream fileOutput;
				std::ostream& o = targetOut(fileOutput, filename);

				for (const auto& input : opt.inputFiles) {
					const auto absolute = std::filesystem::absolute(input);

					if (!std::filesystem::exists(absolute)) {
						std::cerr << "Error: Path does not exist: " << absolute << std::endl;
						continue;
					}

					writeTokenCountTree(o,absolute, opt);
				}
				return;
			}

			std::ofstream fileOutput;

			std::ostream& o = targetOut(fileOutput, filename);
			bool isOnlyDir = opt.dirsOnly;

			for (const auto& input : opt.inputFiles) {
				const auto absolute = std::filesystem::absolute(input);
				o << "## File System Location\n\n" << absolute.string() << "\n\n";
				if (!std::filesystem::exists(absolute)) {
					std::cerr << "Error: Path does not exist: " << absolute << std::endl;
					continue;
				}
				writeGitInfo(o, absolute);
				writeFileStructure(o, absolute);

				if (!isOnlyDir) {
					writeFileContents(o, absolute, opt);
				}
				 writeFileStatistics(o, absolute, opt);
			}

		}
		catch (const std::exception& err) {
			std::cerr << "Error with rendering: " << err.what() << '\n';
		}
	}

}
