#ifndef IOUTILS_HPP_INCLUDED
#define IOUTILS_HPP_INCLUDED
#include <fstream>
using std::ifstream;
using std::ofstream;
#define IFSTREAM_BUFFER_SIZE 1024
#define OFSTREAM_BUFFER_SIZE 1024
class ifstream_buffer {
private:
	ifstream& file;
	char buffer[IFSTREAM_BUFFER_SIZE];
	int cursor;
public:
	ifstream_buffer(ifstream& in): file(in), buffer(), cursor(0) {}
	bool has_next() {
		if(cursor == file.gcount()) {
			file.read(buffer, IFSTREAM_BUFFER_SIZE);
			cursor = 0;
		}
		return cursor < file.gcount();
	}
	char next() {
		return buffer[cursor++];
	}
};
class ofstream_buffer {
private:
	ofstream& file;
	char buffer[OFSTREAM_BUFFER_SIZE];
	int cursor;
public:
	ofstream_buffer(ofstream& out): file(out), buffer(), cursor(0) {}
	~ofstream_buffer() {
		file.write(buffer, cursor);
	}
	void write(char c) {
		if(cursor == OFSTREAM_BUFFER_SIZE) {
			file.write(buffer, OFSTREAM_BUFFER_SIZE);
			cursor = 0;
		}
		buffer[cursor] = c;
		++cursor;
	}
};

#endif // IOUTILS_HPP_INCLUDED

