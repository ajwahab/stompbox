#pragma once

#include <filesystem>

#if _WIN32
#include "windows.h"
#endif

#include <math.h>
#include <algorithm>
#include <memory.h>
#include <sys/stat.h>
#include <fstream>
#include <limits.h>

#include "StompBox.h"

#include "NAM/dsp.h"

enum
{
	NAM_MODEL,
	NAM_NUMPARAMETERS
};


class NAM : public StompBox
{
private:
	double modelIndex = -1;
	int loadedModelIndex = -1;
	std::vector<std::string> modelNames;
	std::vector<std::string> modelPaths;
	std::unique_ptr<DSP> stagedModel;
	std::unique_ptr<DSP> activeModel;
	double prevDCInput = 0;
	double prevDCOutput = 0;

public:

	NAM();
	virtual ~NAM() {}
	virtual void init(int samplingFreq);
	void IndexModels(std::filesystem::path path);
	void SetParameterValue(StompBoxParameter* parameter, double value);
	void SetModel(int index);
	void SetModel(const std::string filename);
	virtual void compute(int count, double* input, double* output);
};