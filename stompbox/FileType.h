#pragma once

#include <string>
#include <filesystem>

class FileType
{
protected:
	std::string folderName;
	std::vector<std::string> fileExtensions;
	std::filesystem::path folderPath;
	std::vector<std::string> fileNames;
	std::vector<std::filesystem::path> filePaths;


public:
	FileType(const std::string folderName, const std::vector<std::string> fileExtensions, const std::filesystem::path& basePath) :
		folderName(folderName),
		fileExtensions(fileExtensions),
		folderPath(basePath / folderName)
	{
		IndexFiles();
	}

	void IndexFiles()
	{
		fileNames.clear();
		filePaths.clear();

		if (!std::filesystem::exists(folderPath))
			std::filesystem::create_directory(folderPath);

		struct stat fstat;

		if (stat(folderPath.string().c_str(), &fstat) == 0)
		{
			for (const auto& entry : std::filesystem::directory_iterator(folderPath))
			{
				auto extension = entry.path().filename().extension();

				for (const auto& fileExtension : fileExtensions)
				{
					if (fileExtension == extension)
					{
						filePaths.push_back(entry.path());

						break;
					}
				}
			}
		}

		std::sort(filePaths.begin(), filePaths.end());

		for (const auto& entry : filePaths)
		{
			auto filename = entry.filename().replace_extension();

			fileNames.push_back(filename.string());
		}
	}

	std::string& GetFolderName()
	{
		return folderName;
	}

	std::vector<std::string>& GetFileNames()
	{
		return fileNames;
	}

	std::vector<std::filesystem::path>& GetFilePaths()
	{
		return filePaths;
	}
};