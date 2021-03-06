/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr) 
 * Unauthorized copying of this file, via any medium is strictly prohibited. 
 * Proprietary and confidential.
 *
 * SystusRunner.cpp
 *
 *  Created on: Nov 8, 2013
 *      Author: devel
 */

#include "SystusRunner.h"
#include "../Abstract/ConfigurationParameters.h"
#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>

namespace vega {

namespace fs = boost::filesystem;
using namespace std;

SystusRunner::SystusRunner() {
	// TODO Auto-generated constructor stub
}

Runner::ExitCode SystusRunner::execSolver(const ConfigurationParameters &configuration,
		string modelFile) {

	fs::path modelFilePath(modelFile);
	string fname = modelFilePath.stem().string();
	string command = "systus -batch -exec " + modelFile + " > " + fname + ".stdout 2> " + fname
			+ ".stderr";
	fs::path outputFsPath(configuration.outputPath);
	cout << configuration.outputPath << endl;
	if (!fs::is_directory(outputFsPath))
		return SOLVER_NOT_FOUND;
	if (configuration.outputPath != ".")
		command = "cd " + configuration.outputPath + " && " + command;

	// delete previous results
	for (fs::directory_iterator it(outputFsPath); it != fs::directory_iterator(); it++) {
		if (fs::is_regular_file(it->status())) {
			string filename = it->path().filename().string();
			if (filename.find(fname) == 0) {
				if (filename.rfind(".TIT") + 4 == filename.size()
						|| filename.rfind(".fdb") + 4 == filename.size()) {
					remove(*it);
				}
			}
			if (filename.find("SYSTUS") == 0 && filename.rfind(".DAT") + 4 == filename.size())
				remove(*it);
		}
	}

	//run command
	int i = system(command.c_str());
	if (i != 0) {
		cerr << "Command " << command << " exit code:" << i << endl;
	} else if (configuration.logLevel >= LogLevel::DEBUG) {
		cout << "Command " << command << " ended with exit code " << i << endl;
	}

	ExitCode exitCode = convertExecResult(i);

	// test if any error occurred during computation
	string outputFileStr = (outputFsPath / fs::path(fname + ".stdout")).string();
	ifstream outputFile(outputFileStr);
	string line;
	int lineNumber = 0;
	while (getline(outputFile, line)) {
		lineNumber += 1;
		if (line.find("ERROR") != string::npos && line.find("NO ERROR") == string::npos){
			cerr << "Error found : line " << lineNumber << " file: " << outputFileStr << endl;
			exitCode = SOLVER_EXIT_NOT_ZERO;
		}
	}

	// test if result files exist
	if (exitCode == OK) {
		for (fs::directory_iterator it(outputFsPath); it != fs::directory_iterator(); it++) {
			if (fs::is_regular_file(it->status())) {
				string filename = it->path().filename().string();
				if (filename.find(fname + "_") == 0 && filename.rfind(".DAT") + 4 == filename.size()) {
					string titFilename = fname + "_DATA" +
							filename.substr(fname.size() + 1, filename.size() - (fname.size() + 1) - 4) + ".TIT";
					string fdbFilename = fname + "_POST" +
							filename.substr(fname.size() + 1, filename.size() - (fname.size() + 1) - 4) + ".fdb";
					string resuFilename = fname + "_" +
							filename.substr(fname.size() + 1, filename.size() - (fname.size() + 1) - 4) + ".RESU";
					if (!(fs::exists(outputFsPath / fs::path(titFilename)) &&
							fs::exists(outputFsPath / fs::path(fdbFilename)) ))
						return SOLVER_RESULT_NOT_FOUND;
					if (fs::exists(outputFsPath / fs::path(resuFilename))){
						//check if it contains nook
						string resuFileStr = (outputFsPath / fs::path(resuFilename)).string();
						string line;
						int lineNumber = 0;
						ifstream resuFile(resuFileStr);
						while (getline(resuFile, line)) {
							lineNumber += 1;
							if (line.find("NOOK") != string::npos) {
								cerr << "Test fail: line " << lineNumber << " file: " << resuFilename << endl;
								exitCode = TEST_FAIL;
							}
						}

					}
				}
			}

		}
	}
	return exitCode;
}

SystusRunner::~SystusRunner() {
	// TODO Auto-generated destructor stub
}

} /* namespace vega */

