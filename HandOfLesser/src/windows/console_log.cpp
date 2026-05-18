#include "console_log.h"
#include "src/core/app_paths.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <streambuf>

namespace
{
	class TeeStreamBuf : public std::streambuf
	{
	public:
		TeeStreamBuf(std::streambuf* primaryBuffer, std::streambuf* secondaryBuffer)
			: mPrimaryBuffer(primaryBuffer), mSecondaryBuffer(secondaryBuffer)
		{
		}

	protected:
		int overflow(int character) override
		{
			if (character == EOF)
			{
				return this->sync() == 0 ? 0 : EOF;
			}

			const int primaryResult = this->mPrimaryBuffer ? this->mPrimaryBuffer->sputc(character) : 0;
			const int secondaryResult
				= this->mSecondaryBuffer ? this->mSecondaryBuffer->sputc(character) : 0;

			if ((this->mPrimaryBuffer && primaryResult == EOF)
				|| (this->mSecondaryBuffer && secondaryResult == EOF))
			{
				return EOF;
			}

			return character;
		}

		int sync() override
		{
			const int primaryResult = this->mPrimaryBuffer ? this->mPrimaryBuffer->pubsync() : 0;
			const int secondaryResult = this->mSecondaryBuffer ? this->mSecondaryBuffer->pubsync() : 0;
			return (primaryResult == 0 && secondaryResult == 0) ? 0 : -1;
		}

	private:
		std::streambuf* mPrimaryBuffer = nullptr;
		std::streambuf* mSecondaryBuffer = nullptr;
	};

	std::ofstream gLogFile;
	TeeStreamBuf* gCoutBuffer = nullptr;
	TeeStreamBuf* gCerrBuffer = nullptr;
	bool gInstalled = false;
} // namespace

void HOL::Windows::ConsoleLog::install()
{
	if (gInstalled)
	{
		return;
	}

	const std::filesystem::path logPath = HOL::Paths::getLogFilePath();
	gLogFile.open(logPath, std::ios::out | std::ios::trunc);
	if (!gLogFile.is_open())
	{
		std::cerr << "Failed to open log file at " << logPath.string() << std::endl;
		return;
	}

	gCoutBuffer = new TeeStreamBuf(std::cout.rdbuf(), gLogFile.rdbuf());
	gCerrBuffer = new TeeStreamBuf(std::cerr.rdbuf(), gLogFile.rdbuf());

	std::cout.rdbuf(gCoutBuffer);
	std::cerr.rdbuf(gCerrBuffer);
	std::cout << std::unitbuf;
	std::cerr << std::unitbuf;

	gInstalled = true;
	std::cout << "Logging console output to " << logPath.string() << std::endl;
}
