#include "Sample.h"

#include <FS/RamFile.h>
#include <Audio/SourceAAC.h>
#include <Audio/SourceWAV.h>

Sample::Sample(fs::File file, bool preload){
	if(!file) return;

	sourceFile = preload ? RamFile::open(file) : file;

	String fname = file.name();
	fname.toLowerCase();

	dataSource = new FileDataSource(sourceFile);

	if(fname.endsWith(".aac") || fname.endsWith(".m4a")){
		source = new SourceAAC(*dataSource);
	}else{
		source = new SourceWAV(*dataSource);
	}

	if(preload){
		file.close();
	}
}

Sample::~Sample(){
	sourceFile.close();
	delete source;
	delete dataSource;
}

Source* Sample::getSource() const{
	return source;
}

bool Sample::isLooping() const{
	return looping;
}

void Sample::setLooping(bool looping){
	Sample::looping = looping;
}
