/*
NotoRaptor (notoraptor@notoraptor.net), Mercredi 26 Février 2014.
CVR : "Chiffre de Vigenère Renforcé".
Algorithme de chiffrement et de déchiffrement de données dérivé du Chiffre de Vigenère.
Version alpha 1. Note : ce code n'est pas optimisé !
Utilisation :
	cvr cheminFichier motDePasse		// Supprime "cheminFichier" et crée "cheminFichier.cvr".
	cvr cheminFichier.cvr motDePasse	// Crée "cheminFichier", ne supprime pas "cheminFichier.cvr".
Licence :
	Citer 'Chiffre de Vigenère Renforcé ("CVR") proposé par NotoRaptor ( http://notoraptor.net ). Contact : notoraptor@notoraptor.net.'. C'est tout ;) !
*/
#include <cstdio>	// Pour lire et écrire dans les fichiers.
#include <climits>	// Pour avoir la taille de l'alphabet (constante UCHAR_MAX).
#include <cstring>	// Pour calculer les longueurs des chaînes de caractères et faire des opérations sur la mémoire.
#include <iostream>	// Pour afficher l'aide du programme et certains messages de débuggage.
#include <sstream>	// Pour écrire plus facilement les messages des exceptions.
#include <string>	// Pour retourner des exceptions.
using namespace std;

/*
Je travaille avec l'alphabet associé au type unsigned char.
En général, il s'agit d'un alphabet de 256 caractères encodés par des valeurs allant de 0 à 255 inclus.
Je préfère les unsigned char aux char car les unsigned char sont des nombres positifs, plus simples à gérer pour les modulo !
Cependant, cela ne pose pas de problème car un caractère c représenté sur un char
est toujours le même lorsqu'il est converti en unsigned char, que c soit positif ou négatif.
	char c;
	unsigned char u = (unsigned char)c;	// Même caractère (mêmes bits), on ne fait que le regarder "différement" !
*/

const size_t tailleAlphabet = (size_t)UCHAR_MAX + 1;

/*
Fonction qui calcule la somme de deux chaînes en base tailleAlphabet.
La fonction conserve seulement les tailleSortie derniers caractères du résultat et les stocke dans le paramètre "sortie".
*/
void sommeSurPlace(unsigned char* sortie, const unsigned char* entree, size_t tailleSortie, int64_t tailleEntree) {
	unsigned int retenue = 0;
	for(size_t i = 0; i < tailleSortie; ++i) {
		unsigned int a = (unsigned int)sortie[tailleSortie - 1 - i];
		unsigned int b = 0;
		if(tailleEntree - 1 - i >= 0) b = (unsigned int)entree[tailleEntree - 1 - i];
		unsigned int c = (a + b + retenue) % tailleAlphabet;
		retenue = (a + b + retenue - c) / tailleAlphabet;
		sortie[tailleSortie - 1 - i] = (unsigned char)c;
	};
};

/*
Classe simple utilisée pour faire des allocations de mémoire.
Son avantage est son destructeur. Ainsi, même si des exceptions sont lancées,
le destructeur est appelé et la mémoire est toujours libérée.
Note : puisque cette classe fait des allocations de mémoire, il faudrait écrire le constructeur de copie et l'opérateur d'affectation.
Mais je ne les ai pas écrits car je ne les utilise jamais dans ce programme !
*/
template<typename T> class Allocateur {
protected:
	T* pointeur;
public:
	Allocateur() {
		pointeur = NULL;
	};
	~Allocateur() {
		delete[] pointeur;
	};
	void allouer(size_t taille) throw(string) {
		if(pointeur != NULL) {
			delete[] pointeur;
			pointeur = NULL;
		};
		try {
			pointeur = new T[taille];
		} catch(bad_alloc& exception) {
			ostringstream chaine;
			chaine << "Erreur d'allocation memoire : " << exception.what();
			throw chaine.str();
		};
	};
	T* memoire() {
		return pointeur;
	};
	T& operator[](int64_t position) {
		return pointeur[position];
	};
};

/*
Classe utilisée pour calculer la "sensibilité" d'une chaîne de caractères x.
x est considérée comme un nombre écrit en base k = tailleAlphabet.
Calcul de la sensibilité pour une chaîne x de taille m :
y = (x^2 mod (k^m)) + plancher(x^2 / (k^m))
En pratique :
-> calculer c = x^2.
-> a = nombre équivalant aux m derniers caractères de c.
-> b = nombre équivalant aux premiers caractères de c (m caractères au maximum).
-> y = a + b
*//*
Note : puisqu'on fait des allocations de mémoire, il faudrait défnir le constructeur de copie et l'opérateur d'affectation.
Cependant, ils ne seront jamais utilisés dans le code de ce programme, donc je ne les ai pas écrits.
*/
class Sensibilite {
protected :
	size_t m;
	unsigned char* sortie;
	unsigned char* table;
	unsigned char* sous_partie_1;
	unsigned char* sous_partie_2;
public :
	void libererMemoire() {
		delete[] sortie;
		delete[] table;
		delete[] sous_partie_1;
		delete[] sous_partie_2;
	};
	Sensibilite(size_t taille) throw(string) {
		if(taille == 0) throw string("On ne peut pas creer une instance de Sensibilite avec une taille nulle.");
		m = taille;
		try {
			sortie = new unsigned char[m + 1];
			table = new unsigned char[2*m*m];
			sous_partie_1 = new unsigned char[m];
			sous_partie_2 = new unsigned char[m];
			memset(sortie, 0, m + 1);
			memset(sous_partie_1, 0, m);
			memset(sous_partie_2, 0, m);
			/* Le tableau "table" n'a pas besoin d'être initialisé ici, car il est initialisé à chaque appel de la méthode calculer(). */
		} catch(bad_alloc& exception) {
			libererMemoire();
			ostringstream chaine;
			chaine << "Erreur survenue lors d'une allocation de memoire dans une instance de Sensibilite : " << exception.what();
			throw chaine.str();
		};
	};
	~Sensibilite() {
		libererMemoire();
	};
	void definir(const char* nombre, size_t longueurNombre) throw(string) {
		if(longueurNombre > m) {
			ostringstream chaine;
			chaine << "Une instance de Sensibilite de taille " << m << " ne peut pas gerer des nombres contenant plus de " << m << " chiffres.";
			throw chaine.str();
		};
		memset(sortie, 0, m + 1 - longueurNombre);
		memcpy(sortie + (m + 1 - longueurNombre), nombre, longueurNombre);
	};
	const unsigned char* actuelle() const {
		return sortie;
	};
	void ajouter(const unsigned char* nombre, size_t taille) {
		sommeSurPlace(sortie, nombre, m + 1, taille);
	};
	void calculer() {
		memset(table, 0, 2*m*m);
		unsigned int retenue;
		for(size_t i = 0; i < m; ++i) {
			retenue = 0;
			unsigned int a = (unsigned int)sortie[m - i];
			for(size_t j = 0; j < m; ++j) {
				unsigned int b = (unsigned int)sortie[m - j];
				unsigned int c = (a*b + retenue) % tailleAlphabet;
				retenue = (a*b + retenue - c) / tailleAlphabet;
				table[i*2*m + (2*m - 1 - i - j)] = (unsigned char)c;
			};
			table[i*2*m + (m - 1 - i)] = (unsigned char)retenue;
		};
		retenue = 0;
		for(size_t i = 2*m - 1; i >= m; --i) {
			uint64_t somme = 0;
			for(size_t j = 0; j < m; ++j) {
				somme += (uint64_t)table[j*2*m + i];
			};
			unsigned int c = (somme + retenue) % tailleAlphabet;
			retenue = (somme + retenue - c) / tailleAlphabet;
			sous_partie_1[i - m] = (unsigned char)c;
		};
		for(int64_t i = m - 1; i >= 0; --i) {
			uint64_t somme = 0;
			for(size_t j = 0; j < m; ++j) {
				somme += (uint64_t)table[j*2*m + i];
			};
			unsigned int c = (somme + retenue) % tailleAlphabet;
			retenue = (somme + retenue - c) / tailleAlphabet;
			sous_partie_2[i] = (unsigned char)c;
		};
		retenue = 0;
		for(int64_t i = m - 1; i >= 0; --i) {
			unsigned int a = (unsigned int)sous_partie_1[i];
			unsigned int b = (unsigned int)sous_partie_2[i];
			unsigned int c = (a + b + retenue) % tailleAlphabet;
			retenue = (a + b + retenue - c) / tailleAlphabet;
			sortie[i + 1] = (unsigned char)c;
		};
		sortie[0] = (unsigned char)retenue;
	};
};

/*
Classe qui représente un mot de passe.
*/
class MotDePasse {
protected:
	const unsigned char* cle;
	size_t longueurCle;
	size_t indice;
public:
	MotDePasse() : cle(NULL), longueurCle(0), indice(0) {};
	void definir(const unsigned char* chaine, size_t depart, size_t taille) throw(string) {
		if(chaine == NULL) throw string("Un mot de passe doit etre une chaine non vide.");
		cle = chaine + depart;
		longueurCle = taille;
		indice = 0;
	};
	unsigned char prochainCaractere() throw(string) {
		if(indice == longueurCle) throw string("Mot de passe vide.");
		return cle[indice++];
	};
	unsigned char caractere(size_t position) throw(string) {
		if(position >= longueurCle) throw string("Position hors des limites dans le mot de passe actuel.");
		return cle[position];
	};
	size_t longueur() const {
		return longueurCle;
	};
	bool vide() const {
		return indice == longueurCle;
	};
};

/*
Classe qui génère la suite de mots de passe à utiliser pendant le chiffrement ou le déchiffrement.
Cette suite est définie comme suit :
	x[0] = mot de passe.
	x[1] = sensibilité sur x[0].
	x[2] = sensibilité sur ((x[0] + x[1])).
	...
	x[9] = sensibilité sur ((x[7] + x[8])).
	pour n > 9, x[n] = (x[n-7] + x[n-10]) mod (k^m).
		k = tailleAlphabet, m = longueur du mot de passe. Donc on garde les m dernieres caractères de (x[n-7] + x[n-10]) dans x[n] pour n > 9.
	Ls suite pour n > 9 est une suite dérivée de la suite de Fibonacci.
	Cette suite m'est suggérée par Wikipiédia comme suite capable de générer des nombres pseudo-aléatoires.
	Selon Wikipédia, le cycle de cette suite est extrêmement grand. Pour plus d'informations, consulter la page Wikipédia :
		http://en.wikipedia.org/wiki/Lagged_Fibonacci_generator
	Idéalement, chaque suite devrait être spécifique à un mot de passe. Cependant, je ne sais pas encore comment vérifier ça ...
	On chiffre avec les termes x[n] pour n > 9.
*/
class MotsDePasse {
protected:
	size_t terme;	// indice n du terme (U[n]) de la suite de mots de passe générés.
	size_t m;
	Allocateur<unsigned char> liste[11];
	Allocateur<unsigned char> actuel;
	MotDePasse motDePasse;
public:
	MotsDePasse(const char* cle) throw(string) : liste(), actuel(), motDePasse() {
		if(cle == NULL || strlen(cle) == 0) throw string("Un mot de passe ne doit pas etre une chaine vide.");
		m = strlen(cle);
		for(int i = 0; i < 11; ++i) liste[i].allouer(m + 17);
		Sensibilite sensibilite(m + 16);
		sensibilite.definir(cle, m);
		sensibilite.calculer();
		memset(liste[0].memoire(), 0, 17);
		memcpy(liste[0].memoire() + 17, cle, m);
		memcpy(liste[1].memoire(), sensibilite.actuelle(), m + 17);
		for(int i = 2; i < 10; ++i) {
			sensibilite.ajouter(liste[i - 2].memoire(), m + 17);
			sensibilite.calculer();
			memcpy(liste[i].memoire(), sensibilite.actuelle(), m + 17);
		};
		memset(liste[10].memoire(), 0, m + 17);
		actuel.allouer(m);
		memset(actuel.memoire(), 0, m);
		terme = 9;
	};
	MotDePasse* prochain() {
		size_t taille = 0;
		size_t entree = 0;
		do {
			++terme;
			entree = terme % 11;
			memcpy(liste[entree].memoire(), liste[(terme - 10) % 11].memoire(), m + 17);
			sommeSurPlace(liste[entree].memoire(), liste[(terme - 7) % 11].memoire(), m + 17, m + 17);
			memset(liste[entree].memoire(), 0, 17);
			size_t vide;
			for(vide = 0; vide < m && liste[entree][vide + 17] == 0; ++vide);
			taille = m - vide;
		} while(taille == 0);
		memcpy(actuel.memoire() + (m - taille), liste[entree].memoire() + (m + 17 - taille), taille);
		motDePasse.definir(actuel.memoire(), m - taille, taille);
		return &motDePasse;
	};
	MotDePasse* motDePasseActuel() {
		return &motDePasse;
	};
};

/*
La classe suivante permet le chiffrement et le déchiffrement proprement dits.
*/
class Chiffrement {
protected:
	const char* chemin;
	MotsDePasse motsDePasse;
public:
	Chiffrement(const char* cheminFichier, const char* cle) throw(string) : chemin(cheminFichier), motsDePasse(cle) {
		if(chemin  == NULL || strlen(chemin) == 0) throw string("Le chemin vers le fichier message est une chaine vide.");
	};
	/*
	Méthode qui vérifie si l'extension d'un fichier est .cvr.
	Si oui, elle enlève l'extension, sinon elle la rajoute, dans le paramètre cheminSortie.
	Un fichier .CVR est supposé être un fichier qui a été chiffré avec ce programme.
	*/
	bool fichierCVR(const char* chemin, Allocateur<char>& cheminSortie) {
		bool test = false;
		size_t longueurChemin = strlen(chemin);
		if(longueurChemin > 4 && (
			(chemin[longueurChemin - 4] == '.') &&
			(chemin[longueurChemin - 3] == 'c' || chemin[longueurChemin - 3] == 'C') &&
			(chemin[longueurChemin - 2] == 'v' || chemin[longueurChemin - 2] == 'V') &&
			(chemin[longueurChemin - 1] == 'r' || chemin[longueurChemin - 1] == 'R')
		)) {
			test = true;
			cheminSortie.allouer(longueurChemin - 3);
			memcpy(cheminSortie.memoire(), chemin, longueurChemin - 4);
			cheminSortie[longueurChemin - 4] = '\0';
		} else {
			cheminSortie.allouer(longueurChemin + 5);
			memcpy(cheminSortie.memoire(), chemin, longueurChemin);
			cheminSortie[longueurChemin + 0] = '.';
			cheminSortie[longueurChemin + 1] = 'c';
			cheminSortie[longueurChemin + 2] = 'v';
			cheminSortie[longueurChemin + 3] = 'r';
			cheminSortie[longueurChemin + 4] = '\0';
		};
		return test;
	};
	/*
	Méthode qui détermine si on chiffre ou si on déchiffre.
	Notons que, dans le cas d'un chiffrement, le fichier initial est supprimé, et seul le fichier chiffré subsiste sur le disque dur.
	Je pars du principe qu'il ne faut pas laisser le fichier à chiffrer en clair une fois que sa version chiffrée est disponible.
	*/
	void executer() throw(string) {
		Allocateur<char> sortie;
		bool avonsCVR = fichierCVR(chemin, sortie);
		if(avonsCVR) {
			dechiffrer(sortie.memoire());
		} else {
			chiffrer(sortie.memoire());
		};
		if(!avonsCVR && remove(chemin) < 0) {
			ostringstream chaine;
			chaine << "Impossible de supprimer le fichier en clair \"" << chemin << "\".";
			throw chaine.str();
		};
	};
	unsigned int pourChiffrer(unsigned int t, unsigned int u, unsigned int v) {
		return (t + (u+v)/2);
	};
	int pourDechiffrer(unsigned int c, unsigned int u, unsigned int v) {
		return (int)(c - (u+v)/2);
	};
	void chiffrer(const char* cheminSortie) throw(string) {
		if(cheminSortie == NULL || strlen(cheminSortie) == 0) throw string("Le chemin vers le fichier de sortie est une chaine vide.");
		FILE* fichier = fopen(chemin, "rb");
		if(fichier == NULL) throw string("Impossible de lire le fichier du message lors d'un chiffrement.");
		FILE* sortie = fopen(cheminSortie, "wb");
		if(sortie == NULL) {
			fclose(fichier);
			ostringstream chaine;
			chaine << "Impossible de creer le fichier de sortie lors d'un chiffrement (" << cheminSortie << ").";
			throw chaine.str();
		};
		int caractere = 0;
		int caracterePrecedent = 0;
		unsigned int j = 0;
		uint64_t sommeQuotients = 0;
		MotDePasse* motDePasse = motsDePasse.motDePasseActuel();
		while((caractere = fgetc(fichier)) != EOF) {
			if(motDePasse->vide()) {
				motDePasse = motsDePasse.prochain();
			};
			j = (j + (unsigned int)caracterePrecedent) % motDePasse->longueur();
			unsigned int t = (unsigned int)caractere;
			unsigned int u = (unsigned int)motDePasse->prochainCaractere();
			unsigned int v = (unsigned int)motDePasse->caractere(j);
			if(u == 0) {
				u = sommeQuotients % tailleAlphabet;
				sommeQuotients = sommeQuotients / tailleAlphabet;
			};
			unsigned int c = pourChiffrer(t,u,v) % tailleAlphabet;
			sommeQuotients += pourChiffrer(t,u,v) / tailleAlphabet;
			fputc(c, sortie);
			caracterePrecedent = caractere;
		};
		fclose(fichier);
		fclose(sortie);
	};
	void dechiffrer(const char* cheminSortie) throw(string) {
		if(cheminSortie == NULL || strlen(cheminSortie) == 0) throw string("Le chemin vers le fichier de sortie est une chaine vide.");
		FILE* fichier = fopen(chemin, "rb");
		if(fichier == NULL) throw string("Impossible de lire le fichier du message lors d'un dechiffrement.");
		FILE* sortie = fopen(cheminSortie, "wb");
		if(sortie == NULL) {
			fclose(fichier);
			ostringstream chaine;
			chaine << "Impossible de creer le fichier de sortie lors d'un dechiffrement (" << cheminSortie << ").";
			throw chaine.str();
		};
		int caractere = 0;
		int caracterePrecedent = 0;
		unsigned int j = 0;
		uint64_t sommeQuotients = 0;
		MotDePasse* motDePasse = motsDePasse.motDePasseActuel();
		while((caractere = fgetc(fichier)) != EOF) {
			if(motDePasse->vide()) {
				motDePasse = motsDePasse.prochain();
			};
			j = (j + (unsigned int)caracterePrecedent) % motDePasse->longueur();
			unsigned int c = (unsigned int)caractere;
			unsigned int u = (unsigned int)motDePasse->prochainCaractere();
			unsigned int v = (unsigned int)motDePasse->caractere(j);
			if(u == 0) {
				u = sommeQuotients % tailleAlphabet;
				sommeQuotients = sommeQuotients / tailleAlphabet;
			};
			unsigned int q = (tailleAlphabet - 1 - pourDechiffrer(c,u,v))/tailleAlphabet;
			unsigned int t = tailleAlphabet*q + pourDechiffrer(c,u,v);
			sommeQuotients += q;
			fputc(t, sortie);
			caracterePrecedent = t;
		};
		fclose(fichier);
		fclose(sortie);
	};
};

int main(int narg, char* targ[]) {
	if(narg > 2) {
		try {
			const char* chemin = targ[1];
			const char* cle = targ[2];
			Chiffrement chiffrement(chemin, cle);
			chiffrement.executer();
		} catch(string& s) {
			cerr << s << endl;
			return 1;
		};
	} else {
		cout << "Utilisation :" << endl;
		cout << "\tPour chiffrer :" << endl;
		cout << "\t\t" << targ[0] << " <fichier-non-cvr> <mot-de-passe>" << endl;
		cout << "\tPour dechiffrer :" << endl;
		cout << "\t\t" << targ[0] << " <fichier-cvr> <mot-de-passe>" << endl;
		cout << "\tUn fichier CVR est un fichier qui a l'extension \".cvr\". il s'agit normalement d'un fichier chiffre par ce programme." << endl;
		return 1;
	};
	return 0;
};

