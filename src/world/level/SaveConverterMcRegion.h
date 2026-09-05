#pragma once

#include "java/String.h"

class File;
class ProgressListener;

class SaveConverterMcRegion
{
private:
	jstring savesDirectory;

public:
	explicit SaveConverterMcRegion(File &savesDirectory);

	jstring getFormatName() const;
	bool isOldMapFormat(const jstring &name) const;
	bool convertMapFormat(const jstring &name, ProgressListener &progress);
};
