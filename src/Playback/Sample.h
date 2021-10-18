#ifndef BYTEBOI_LIBRARY_SAMPLE_H
#define BYTEBOI_LIBRARY_SAMPLE_H

#include <CMAudio.h>
#include <Audio/Source.h>

class Sample {
public:
	Sample(fs::File file, bool preload = false);
	virtual ~Sample();

	Source* getSource() const;

private:
	Source* source = nullptr;
};


#endif //BYTEBOI_LIBRARY_SAMPLE_H
