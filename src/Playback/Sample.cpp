#include "Sample.h"

#include <FS/RamFile.h>
#include <Audio/SourceAAC.h>
#include <Audio/SourceWAV.h>

Sample::Sample(fs::File file, bool preload){
	fs::File sourceFile = preload ? RamFile::open(file) : file;

	String fname = file.name();
	fname.toLowerCase();

	if(fname.endsWith(".aac") || fname.endsWith(".m4a")){
		source = new SourceAAC(sourceFile);
	}else{
		source = new SourceWAV(sourceFile);
	}
}

Sample::~Sample(){
	delete source;
}

Source* Sample::getSource() const{
	return source;
}
