#include <cstring>	// Pour calculer les longueurs des chaînes de caractères et faire des opérations sur la mémoire.
#include <sstream>	// Pour écrire plus facilement les messages des exceptions.
#include <limits>
#include "ioutils.hpp"
#include "cvr.hpp"
using namespace std;
//#define DEBUG
#ifdef DEBUG
#include <iostream>
#endif // DEBUG

/* REMARQUES GÉNÉRALES
RAPPEL: l'opérateur new émet une exception std::bad_alloc (dérivée de std::exception) si la création d'un objet échoue.
*/

namespace CVR {
/* Je travaille avec l'alphabet associé au type unsigned char.
En général, il s'agit d'un alphabet de 256 caractères encodés par des valeurs allant de 0 à 255 inclus.
Je préfère les unsigned char aux char car les unsigned char sont des nombres positifs, plus simples à gérer pour les modulo!
Cependant, cela ne pose pas de problème car un caractère c représenté sur un char est toujours le même lorsqu'il est converti en unsigned char, que c soit positif ou négatif.
	char c;
	unsigned char u = (unsigned char)c;	// Même caractère (mêmes bits), seule son interprétation change !
*/
const size_t alphabetSize = (size_t)numeric_limits<unsigned char>::max() + 1;
const uint64_t SIZE_T_MAX = numeric_limits<size_t>::max();

/* Fonction qui calcule la somme de deux chaînes en base alphabetSize.
La fonction conserve seulement les outputSize derniers caractères du résultat et les écrit dans le paramètre "output".
output = (output + input) mod (alphabetSize^strlen(output))
*/
unsigned int sum(unsigned char* output, const unsigned char* input, size_t outputSize, int64_t inputSize) {
	unsigned int retenue = 0;
	for(size_t i = 0; i < outputSize; ++i) {
		unsigned int a = (unsigned int)output[outputSize - 1 - i];
		unsigned int b = 0;
		if(inputSize - 1 - i >= 0) b = (unsigned int)input[inputSize - 1 - i];
		unsigned int c = (a + b + retenue) % alphabetSize;
		retenue        = (a + b + retenue) / alphabetSize;
		output[outputSize - 1 - i] = (unsigned char)c;
	};
	return retenue;
};
/* Version modifiée de la fonction précédente, qui pourrait permettre d'éviter les mem-set pour les opérations d'addition.
PS: Pour le moment, la fonction est utilisée une seule fois dans le code.
Elle sera améliorée et intégrée dans les prochaines versions! */
unsigned int enhancedSum(unsigned char* output, size_t outputShift, size_t outputSize, const unsigned char* input, size_t inputSize) {
	// On fait les suppositions suivantes:
	// outputShift < outputSize.
	// outputSize >= inputSize
	// Le nombre dans output commence à partir de output[outputShift] et va jusqu'à output[outputSize - 1]
	size_t outputNumberLength = outputSize - outputShift;
	size_t commonLength;
	size_t longestNumberRemainingLength = 0;
	const unsigned char* longestNumber = nullptr;
	if(outputNumberLength == inputSize) {
		commonLength = outputNumberLength;
	} else if(outputNumberLength > inputSize) {
		commonLength = inputSize;
		longestNumber = output + outputShift;
		longestNumberRemainingLength = outputNumberLength - commonLength;
	} else {
		commonLength = outputNumberLength;
		longestNumber = input;
		longestNumberRemainingLength = inputSize - commonLength;
	}
	size_t outputRemainingLength = outputSize - commonLength;
	unsigned int retenue = 0;
	for(size_t i = 0; i < commonLength; ++i) {
		unsigned int a = (unsigned int)output[outputSize - 1 - i];
		unsigned int b = (unsigned int)input[inputSize - 1 - i];
		unsigned int c = (a + b + retenue) % alphabetSize;
		retenue        = (a + b + retenue) / alphabetSize;
		output[outputSize - 1 - i] = (unsigned char)c;
	}
	if(outputRemainingLength) {
		if(longestNumber) {
			for(size_t i = 0; i < longestNumberRemainingLength; ++i) {
				unsigned int a = (unsigned int)longestNumber[longestNumberRemainingLength - 1 - i];
				unsigned int c = (a + retenue) % alphabetSize;
				retenue        = (a + retenue) / alphabetSize;
				output[outputSize - 1 - commonLength - i] = (unsigned char)c;
			}
		}
		if(retenue && outputSize - commonLength - longestNumberRemainingLength > 0) {
			output[outputSize - 1 - commonLength - longestNumberRemainingLength] = retenue;
			retenue = 0;
		}
	}
	return retenue;
}

/* Classe simple utilisée pour faire des allocations de mémoire.
Son avantage est son destructeur. Ainsi, même si des exceptions sont lancées,
le destructeur est appelé et la mémoire est toujours libérée.
Note : puisque cette classe fait des allocations de mémoire,
il faudrait écrire le constructeur de copie et l'opérateur d'affectation.
Mais je ne les ai pas écrits car je ne les utilise jamais dans ce programme!
*/
template<typename T> class Memory {
private:
	T* pointeur;
	size_t tailleMemoire;
	size_t padding;
public:
	inline Memory(): pointeur(nullptr), tailleMemoire(0), padding(0) {};
	inline ~Memory() { delete[] pointeur; };
	inline void allocate(size_t taille) {
		if(pointeur) {
			delete[] pointeur;
		};
		pointeur = new T[taille];
		tailleMemoire = taille;
		padding = taille;
	};
	inline T* pointer() {
		return pointeur;
	};
	inline T& operator[](int64_t position) {
		return pointeur[position];
	};
	size_t getSize() const {
		return tailleMemoire;
	}
	size_t getPadding() const {
		return padding;
	}
	void setPadding(size_t newPadding) {
		padding = newPadding;
	}
};

/* Classe utilisée pour calculer la "sensibilité" d'une chaîne de caractères x.
x est considérée comme un nombre écrit en base k = alphabetSize.
Calcul de la sensibilité pour une chaîne x de taille m:
y = (x^2 mod (k^m)) + plancher(x^2 / (k^m))
En pratique :
-> calculer c = x^2.
-> a = nombre équivalant aux m derniers caractères de c.
-> b = nombre équivalant aux premiers caractères de c (m caractères au maximum).
-> y = a + b
*//*
NB: puisqu'on fait des allocations de mémoire, il faudrait défnir le constructeur de copie et l'opérateur d'affectation.
Cependant, ils ne seront jamais utilisés dans le code de ce programme, donc je ne les ai pas écrits.
*/
class Sensitivity {
private:
	size_t m;						// longueur standard de la mémoire utilisée. NB: On suppose qu'on a toujours m != 0
	unsigned char* output;			// taille m+1
	unsigned char* accumulator;		// taille 2m+1
	unsigned char* buffer;			// taille m+2
	size_t currentNumberLength;	// longueur du nombre actuellement dans la output.
	void freeMemory() {
		delete[] output;
		delete[] accumulator;
		delete[] buffer;
	};
public:
	Sensitivity(const char* initialNumber, size_t initialNumberLength, size_t supplement): m(initialNumberLength + supplement) {
		uint64_t greatestAllocated = uint64_t(m) * 2 + 1;
		if(greatestAllocated > SIZE_T_MAX) {
			ostringstream o;
			o << "L'implementation actuelle de Sensitivity ne peut pas gerer une taille de " << m <<
			" chiffres car elle ne peut allouer la plus grande taille (" << greatestAllocated << ") necessaire aux calculs.";
			throw o.str();
		}
		try {
			output     = new unsigned char[m + 1];
			accumulator = new unsigned char[2 * m + 1];
			buffer     = new unsigned char[m + 2];
		} catch(bad_alloc& exception) {
			freeMemory();
			ostringstream o;
			o << "Erreur survenue lors d'une allocation de memoire dans une instance de Sensitivity: " << exception.what();
			throw o.str();
		};
		/// Cette copie de mémoire est nécessaire (copie du nombre à traiter dans l'objet courant) !
		memcpy(output + (supplement + 1), initialNumber, initialNumberLength);
		currentNumberLength = initialNumberLength;
		/* NB: "accumulator" et "buffer" n'ont pas besoin d'être initialisés ici
		car ils sont initialisés ou écrasés à chaque appel de la méthode compute(). */
	};
	~Sensitivity() {
		freeMemory();
	};
	void add(Memory<unsigned char>& memory) {
		const unsigned char* number = memory.pointer() + memory.getPadding();
		size_t addedNumberLength = memory.getSize() - memory.getPadding();
		size_t consideredLength = currentNumberLength;
		if(addedNumberLength > currentNumberLength) {
			if(addedNumberLength > m) {
				ostringstream o;
				o << "Impossible d'ajouter un nombre de longueur " << addedNumberLength << " a une Sensitivity de taille standard " << m << ".";
				throw o.str();
			}
			consideredLength = addedNumberLength;
		}
		//unsigned int retenue = sum(output + (m + 1 - consideredLength), number, consideredLength, addedNumberLength);
		unsigned int retenue = enhancedSum (
			output + (m + 1 - consideredLength), consideredLength - currentNumberLength, consideredLength, number, addedNumberLength
		);
		currentNumberLength = consideredLength;
		if(retenue) {
			if(consideredLength == m) {
				ostringstream o;
				o << "Le resultat d'un ajout dans une Sensitivity est plus long que la taille standard (" << m << ") de cette Sensitivity.";
				throw o.str();
			}
			output[m - consideredLength] = (unsigned char)retenue;
			++currentNumberLength;
		}
	};
	void compute() {
		if(currentNumberLength > m) {
			ostringstream o;
			o << "Impossible de calculer la Sensitivity pour un nombre de taille " << currentNumberLength <<
			 " avec une memoire de taille standard " << m << ".";
			throw o.str();
		}
		memset(accumulator, 0, 2*m+1);
		// Pas besoin d'initialiser "buffer" car tous ses octets sont toujours réécrits avant d'être utilisés à chaque opération.
		unsigned int retenue;
		for(size_t i = 0; i < currentNumberLength; ++i) {
			// Multiplication.
			retenue = 0;
			unsigned int a = (unsigned int)output[m - i];
			for(size_t j = 0; j < currentNumberLength; ++j) {
				unsigned int b = (unsigned int)output[m - j];
				unsigned int c = (a*b + retenue) % alphabetSize;
				retenue        = (a*b + retenue) / alphabetSize;
				buffer[m + 1 - j] = (unsigned char)c;
			};
			buffer[m + 1 - currentNumberLength] = (unsigned char)retenue;
			buffer[m + 1 - currentNumberLength - 1] = 0;
			// Addition.
			retenue = 0;
			for(size_t j = 0; j < currentNumberLength + 2; ++j) {
				unsigned int u = (unsigned int)accumulator[2*m - i - j];
				unsigned int v = (unsigned int)buffer[m + 1 - j];
				unsigned int w = (u + v + retenue) % alphabetSize;
				retenue        = (u + v + retenue) / alphabetSize;
				accumulator[2*m - i - j] = (unsigned char)w;
			}
		};
		unsigned char* sous_partie_1 = accumulator + (2*m + 1 - 2*currentNumberLength);
		unsigned char* sous_partie_2 = accumulator + (2*m + 1 -   currentNumberLength);
		retenue = 0;
		for(size_t i = 0; i < currentNumberLength; ++i) {
			unsigned int a = (unsigned int)sous_partie_1[currentNumberLength - 1 - i];
			unsigned int b = (unsigned int)sous_partie_2[currentNumberLength - 1 - i];
			unsigned int c = (a + b + retenue) % alphabetSize;
			retenue        = (a + b + retenue) / alphabetSize;
			output[m - i] = (unsigned char)c;
		};
		if(retenue) {
			output[m - currentNumberLength] = (unsigned char)retenue;
			++currentNumberLength;
		}
	}
	void copyTo(Memory<unsigned char>& memory) {
		unsigned char* pointer = memory.pointer();
		size_t memorySize = memory.getSize();
		if(memorySize > currentNumberLength) {
			memset(pointer, 0, memorySize - currentNumberLength);
			memcpy(
				pointer + (memorySize - currentNumberLength),
				output + (m + 1 - currentNumberLength),
				currentNumberLength
			);
			memory.setPadding(memorySize - currentNumberLength);
		} else {
			memcpy(pointer, output + (m + 1 - memorySize), memorySize);
			memory.setPadding(0);
		}
	}
};

/* Classe qui représente un mot de passe.
NB: Classe non sûre (pas de vérifications faites lors des accès aux caractères du mot de passe)
car utilisée uniquement dans le contexte de ce fichier-ci.
*/
class Password {
protected:
	const unsigned char* key;
	size_t longueurCle;
	size_t indice;
public:
	inline Password() : key(nullptr), longueurCle(0), indice(0) {};
	inline void setPassword(const unsigned char* chaine, size_t depart, size_t taille) {
		key = chaine + depart;
		longueurCle = taille;
		indice = 0;
	};
	inline unsigned char nextCharacter() {
		return key[indice++];
	};
	inline unsigned char character(size_t position) {
		return key[position];
	};
	inline size_t length() const {
		return longueurCle;
	};
	inline bool isEmpty() const {
		return indice == longueurCle;
	};
};

/* Classe qui génère la suite de mots de passe à utiliser pendant le chiffrement ou le déchiffrement.
Cette suite est définie comme suit :
	x[0] = mot de passe.
	x[1] = sensibilité sur x[0].
	x[2] = sensibilité sur ((x[0] + x[1])).
	...
	x[9] = sensibilité sur ((x[7] + x[8])).
	pour n > 9, x[n] = (x[n-7] + x[n-10]) mod (k^m).
		k = alphabetSize, m = longueur du mot de passe.
		Donc on garde les m dernieres caractères de (x[n-7] + x[n-10]) dans x[n] pour n > 9.
	Ls suite pour n > 9 est une suite dérivée de la suite de Fibonacci.
	Cette suite m'est suggérée par Wikipiédia comme suite capable de générer des nombres pseudo-aléatoires.
	Selon Wikipédia, le cycle de cette suite est extrêmement grand. Pour plus d'informations, consulter la page Wikipédia:
		http://en.wikipedia.org/wiki/Lagged_Fibonacci_generator
	Idéalement, chaque suite devrait être spécifique à un mot de passe. Cependant, je ne sais pas encore comment en être sûr ...
	On chiffre avec les termes x[n] pour n > 9 (donc à partir de x[10]).
	On saute les termes nuls (tout terme x[n] == 0).
*/
class PasswordChain {
protected:
	size_t term;	// Indice n du terme (U[n]) de la suite de mots de passe générés.
	size_t m;		// Longueur du mot de passe.
	Memory<unsigned char> bufferList[11];
	Password password;
public:
	PasswordChain(const char* key);
	Password* next();
	inline Password* currentPassword() {
		return &password;
	};
};
//#define RPADDING 17 // minimum 10 (car 9 calculs de sensibilité + 1 espace de précaution pour assurer les calculs des carrés)
#define RPADDING 10
PasswordChain::PasswordChain(const char* key): bufferList(), password() {
	if(key == nullptr || (m = strlen(key)) == 0) throw string("Un mot de passe ne doit pas etre une chaine vide.");
	for(int i = 0; i < 11; ++i) bufferList[i].allocate(m + RPADDING);
	Sensitivity sensitivity(key, m, RPADDING - 1);
	sensitivity.compute();
	memset(bufferList[0].pointer(), 0, RPADDING);
	memcpy(bufferList[0].pointer() + RPADDING, key, m);
	bufferList[0].setPadding(RPADDING);
	sensitivity.copyTo(bufferList[1]);
	for(int i = 2; i < 10; ++i) {
		sensitivity.add(bufferList[i - 2]);
		sensitivity.compute();
		sensitivity.copyTo(bufferList[i]);
	};
	term = 9;
	#ifdef DEBUG
	for(size_t i = 0; i < 10; ++i) {
		for(size_t j = 0; j < m + RPADDING; ++j) {
			if(j > 0) cout << '\t';
			cout << (unsigned int)(bufferList[i][j]);
		}
		cout << endl;
	}
	#endif // DEBUG
};
Password* PasswordChain::next() {
	size_t taille = 0;
	size_t entry = 0;
	do {
		++term;
		entry = term % 11;
		/// Cette copie de mémoire est nécessaire (copie pour calculer un nouveau terme d'une liste) !
		memcpy(bufferList[entry].pointer(), bufferList[(term - 10) % 11].pointer(), m + RPADDING);
		sum(bufferList[entry].pointer(), bufferList[(term - 7) % 11].pointer(), m + RPADDING, m + RPADDING);
		size_t gap;
		for(gap = 0; gap < m && bufferList[entry][gap + RPADDING] == 0; ++gap);
		taille = m - gap;
	} while(taille == 0); // On saute les valeurs nulles de la suite (n tel que x[n] == 0).
	password.setPassword(bufferList[entry].pointer(), m + RPADDING - taille, taille);
	return &password;
};

inline unsigned int forEncryption(unsigned int t, unsigned int u, unsigned int v) {
	return (t + (u+v)/2);
};
inline int forDecryption(unsigned int c, unsigned int u, unsigned int v) {
	return (int)(c - (u+v)/2);
};
void encrypt(const char* key, ifstream& infile, ofstream& outfile) {
	PasswordChain passwordChain(key);
	int caracterePrecedent = 0;
	unsigned int j = 0;
	uint64_t sommeQuotients = 0;
	Password* password = passwordChain.currentPassword();
	ifstream_buffer in(infile);
	ofstream_buffer out(outfile);
	while(in.has_next()) {
		unsigned char caractere = in.next();
		if(password->isEmpty())
			password = passwordChain.next();
		j = (j + (unsigned int)caracterePrecedent) % password->length();
		unsigned int t = (unsigned int)caractere;
		unsigned int u = (unsigned int)password->nextCharacter();
		unsigned int v = (unsigned int)password->character(j);
		if(u == 0) {
			u = sommeQuotients % alphabetSize;
			sommeQuotients = sommeQuotients / alphabetSize;
		};
		unsigned int c = forEncryption(t,u,v) % alphabetSize;
		sommeQuotients += forEncryption(t,u,v) / alphabetSize;
		out.write((char)c);
		caracterePrecedent = caractere;
	};
};
void decrypt(const char* key, ifstream& infile, ofstream& outfile) {
	PasswordChain passwordChain(key);
	int caracterePrecedent = 0;
	unsigned int j = 0;
	uint64_t sommeQuotients = 0;
	Password* password = passwordChain.currentPassword();
	ifstream_buffer in(infile);
	ofstream_buffer out(outfile);
	while(in.has_next()) {
		unsigned char caractere = in.next();
		if(password->isEmpty())
			password = passwordChain.next();
		j = (j + (unsigned int)caracterePrecedent) % password->length();
		unsigned int c = (unsigned int)caractere;
		unsigned int u = (unsigned int)password->nextCharacter();
		unsigned int v = (unsigned int)password->character(j);
		if(u == 0) {
			u = sommeQuotients % alphabetSize;
			sommeQuotients = sommeQuotients / alphabetSize;
		};
		unsigned int q = (alphabetSize - 1 - forDecryption(c,u,v))/alphabetSize;
		unsigned int t = alphabetSize*q + forDecryption(c,u,v);
		sommeQuotients += q;
		out.write((char)t);
		caracterePrecedent = t;
	};
};

}

