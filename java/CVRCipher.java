package liber.security.cvr;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.nio.charset.StandardCharsets;

class CVRCipher {
	static final int alphabetSize = Byte.MAX_VALUE - Byte.MIN_VALUE + 1;
	static long toEncrypt(long t, long u, long v) {
		return (t + (u + v) / 2);
	}
	static int toDecrypt(long c, long u, long v) {
		return (int) (c - (u + v) / 2);
	}
	static void sumOnSite(UnsignedBytes sortie, UnsignedBytes entree, int tailleSortie, int tailleEntree) {
		long retenue = 0;
		for (int i = 0; i < tailleSortie; ++i) {
			long a = sortie.get(tailleSortie - 1 - i);
			long b = 0;
			if (tailleEntree - 1 - i >= 0)
				b = entree.get(tailleEntree - 1 - i);
			long c = (a + b + retenue) % alphabetSize;
			retenue = (a + b + retenue - c) / alphabetSize;
			sortie.set(tailleSortie - 1 - i, c);
		}
	}
	private String path;
	private CVRPasswords cvrPasswords;
	public CVRCipher(String cheminFichier, String cle) throws Exception {
		assert cheminFichier != null && cheminFichier.length() != 0;
		assert cle != null && cle.length() != 0;
		path = cheminFichier;
		cvrPasswords = new CVRPasswords(new UnsignedBytes(cle.getBytes(StandardCharsets.UTF_8)));
	}
	public void execute() throws Exception {
		StringBuilder sortie = new StringBuilder();
		boolean avonsCVR = CVR.isCVRFile(path, sortie);
		if (avonsCVR) {
			decrypt(sortie.toString());
		} else {
			encrypt(sortie.toString());
			if (!(new File(path).delete()))
				throw new Exception("Impossible de supprimer le fichier en clair \"" + path + "\".");
		}
	}
	private void encrypt(String cheminSortie) throws Exception {
		assert cheminSortie != null && cheminSortie.length() != 0;
		try (
				FileInputStream fichier = new FileInputStream(path);
				FileOutputStream sortie = new FileOutputStream(cheminSortie)
		) {
			int caractere;
			int caracterePrecedent = 0;
			int j = 0;
			long sommeQuotients = 0;
			CVRPassword cvrPassword = cvrPasswords.currentPassword();
			while ((caractere = fichier.read()) != -1) {
				if (cvrPassword.isEmpty()) {
					cvrPassword = cvrPasswords.next();
				}
				j = (j + caracterePrecedent) % cvrPassword.length();
				long t = caractere;
				long u = cvrPassword.nextCharacter();
				long v = cvrPassword.character(j);
				if (u == 0) {
					u = sommeQuotients % alphabetSize;
					sommeQuotients = sommeQuotients / alphabetSize;
				}
				int c = (int) (toEncrypt(t, u, v) % alphabetSize);
				sommeQuotients += toEncrypt(t, u, v) / alphabetSize;
				sortie.write(c);
				caracterePrecedent = caractere;
			}
		}
	}
	private void decrypt(String cheminSortie) throws Exception {
		assert cheminSortie != null && cheminSortie.length() != 0;
		try (
				FileInputStream fichier = new FileInputStream(path);
				FileOutputStream sortie = new FileOutputStream(cheminSortie)
		) {
			int caractere;
			int caracterePrecedent = 0;
			int j = 0;
			long sommeQuotients = 0;
			CVRPassword cvrPassword = cvrPasswords.currentPassword();
			while ((caractere = fichier.read()) != -1) {
				if (cvrPassword.isEmpty()) {
					cvrPassword = cvrPasswords.next();
				}
				j = (j + caracterePrecedent) % cvrPassword.length();
				long c = caractere;
				long u = cvrPassword.nextCharacter();
				long v = cvrPassword.character(j);
				if (u == 0) {
					u = sommeQuotients % alphabetSize;
					sommeQuotients = sommeQuotients / alphabetSize;
				}
				int q = ((alphabetSize - 1 - toDecrypt(c, u, v)) / alphabetSize);
				int t = alphabetSize * q + toDecrypt(c, u, v);
				sommeQuotients += q;
				sortie.write(t);
				caracterePrecedent = t;
			}
		}
	}
}
