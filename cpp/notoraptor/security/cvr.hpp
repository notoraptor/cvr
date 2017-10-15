/*
NotoRaptor (notoraptor@notoraptor.net), Dimanche 4 Septembre 2016.
CVR: "Chiffre de Vigen�re Renforc�". Algorithme de chiffrement sym�trique d�riv� du Chiffre de Vigen�re.
Version alpha 2.
Article au sujet de l'algorithme:
	http://notoraptor.net/2014/02/26/chiffre-de-vigenere-renforce-cvr-technique-de-chiffrement-basee-sur-une-modification-du-chiffre-de-vigenere/
Notes:
	Code �crit par un francophone avec des identifiants en fran�ais.
Licence: utilisation totalement libre ;), simplement citer:
	"Chiffre de Vigen�re Renforc� ("CVR") propos� par NotoRaptor (http://notoraptor.net). Contact: notoraptor@notoraptor.net."
Le code source est dans le fichier "cvr.cpp"!
*/
#ifndef CVR_HPP_INCLUDED
#define CVR_HPP_INCLUDED
#include <fstream>
namespace CVR {
	void encrypt(const char* cle, std::ifstream& input, std::ofstream& output);
	void decrypt(const char* cle, std::ifstream& input, std::ofstream& output);
};
#endif // CVR_HPP_INCLUDED
