#include <iostream>
#include "notoraptor/security/cvr.hpp"
using namespace std;

bool endsWith(const string& str, const string& end) {
	return str.rfind(end) == str.length() - end.length();
}

int main(int n, char* args[]) {
	if(n > 2) {
		try {
			string inputFile = args[1];
			const char* mdp = args[2];
			string extension = ".cvr";
			bool toEncrypt(true);
			string outputFile;
			if(endsWith(inputFile, extension)) {
				toEncrypt = false;
				outputFile = inputFile.substr(0, inputFile.length() - extension.length());
			} else if(n > 3) {
				string specialIdentifier;
				specialIdentifier.append("-").append(args[3]);
				size_t position = inputFile.find('.');
				if(position == string::npos)
					outputFile = inputFile + specialIdentifier + extension;
				else
					outputFile = inputFile.substr(0, position) + specialIdentifier + inputFile.substr(position) + extension;
			} else {
				outputFile = inputFile + extension;
			}
			ifstream input(inputFile, ios::binary);
			ofstream output(outputFile, ios::binary);
			if(toEncrypt)
				CVR::encrypt(mdp, input, output);
			else
				CVR::decrypt(mdp, input, output);
			output.close();
			input.close();
		} catch(exception& e) {
			cout << e.what() << endl;
		} catch(string& s) {
			cout << s << endl;
		}
	} else {
		cout << "Usage:" << endl << endl;
		cout << args[0] << " <file-to-encrypt>     <password> [suffix]" << endl;
		cout << args[0] << " <file-to-decrypt.cvr> <password>" << endl << endl;
		cout << "(Any file with \".cvr\" extension is considered as encrypted, so the program will try to decrypt it. The output filename is the input filename less the \".cvr\" terminal extension.)" << endl;
		cout << "(Any other file will be encrypted. The output filename is the input filename (with suffix, if given, at the end of the basename) plus \".cvr\" extension.)" << endl;
	}
	return 0;
}

