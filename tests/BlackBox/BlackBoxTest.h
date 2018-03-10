#pragma once

extern wstring g_SourceTmpPath;


struct Scenario
{
//	vector<fs::path> m_FiltredFiles;
	MyBCopyFile m_MyBCopyFile;

	void CreateBackup();

	virtual void Execute() = 0;	
	

	virtual void ValidateFiles();
	virtual void PrepareFiles() {	}
	virtual void SaveSrcXML() {	}
};

struct RegularScenario : public Scenario
{		
	const unsigned int cm_NumIteration = 5;
	virtual void Execute() override;
	virtual void PrepareFiles() override;
	virtual void SaveSrcXML() override;	
	
private:
	void CheckRegularFiles();
};

struct IncrementsScenarion : public Scenario
{
	virtual void Execute() override;	

	const unsigned int cm_NumStoredOldRegularBackups = 2;
	const unsigned int cm_LengthIncrementChain = 3;
	void CheckFiles();
};


const vector< shared_ptr<Scenario> > g_Scenarios = 
{
	  make_shared<RegularScenario>()
	, make_shared<IncrementsScenarion>()
	
};



void GenerateRandomFiles(const vector<wstring>& files);
void GenerateRandomData(string& result, size_t length = RandomInt(0, 64 * 1024));
void WriteNewFile(const wstring& file, const string& fileData);
void PreprocessingSrcXml(string& line);


