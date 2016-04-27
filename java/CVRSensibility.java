package liber.security.cvr;

class CVRSensibility {
	private int m;
	private UnsignedBytes output;
	private UnsignedBytes table;
	private UnsignedBytes sub_part_1;
	private UnsignedBytes sub_part_2;
	public CVRSensibility(int taille) {
		assert taille != 0;
		m = taille;
		output = new UnsignedBytes(m + 1);
		table = new UnsignedBytes(2 * m * m);
		sub_part_1 = new UnsignedBytes(m);
		sub_part_2 = new UnsignedBytes(m);
	}
	public void define(UnsignedBytes nombre, int longueurNombre) throws Exception {
		if (longueurNombre > m)
			throw new Exception("Une instance de Sensibilite de taille " + m + " ne peut pas gerer des nombres contenant plus de " + m + " chiffres.");
		output.memclear(m + 1 - longueurNombre);
		output.copy((m + 1 - longueurNombre), nombre, longueurNombre);
	}
	public UnsignedBytes current() {
		return output;
	}
	public void add(UnsignedBytes nombre, int taille) {
		CVRCipher.sumOnSite(output, nombre, m + 1, taille);
	}
	public void compute() {
		table.memclear(2 * m * m);
		long retenue;
		for (int i = 0; i < m; ++i) {
			retenue = 0;
			long a = output.get(m - i);
			for (int j = 0; j < m; ++j) {
				long b = output.get(m - j);
				long c = (a * b + retenue) % CVRCipher.alphabetSize;
				retenue = (a * b + retenue - c) / CVRCipher.alphabetSize;
				table.set((i * 2 * m + (2 * m - 1 - i - j)), c);
			}
			table.set((i * 2 * m + (m - 1 - i)), retenue);
		}
		retenue = 0;
		for (int i = 2 * m - 1; i >= m; --i) {
			long somme = 0;
			for (int j = 0; j < m; ++j) {
				somme += table.get(j * 2 * m + i);
			}
			long c = (somme + retenue) % CVRCipher.alphabetSize;
			retenue = (somme + retenue - c) / CVRCipher.alphabetSize;
			sub_part_1.set((i - m), c);
		}
		for (int i = m - 1; i >= 0; --i) {
			long somme = 0;
			for (int j = 0; j < m; ++j) {
				somme += table.get(j * 2 * m + i);
			}
			long c = (somme + retenue) % CVRCipher.alphabetSize;
			retenue = (somme + retenue - c) / CVRCipher.alphabetSize;
			sub_part_2.set(i, c);
		}
		retenue = 0;
		for (int i = m - 1; i >= 0; --i) {
			long a = sub_part_1.get(i);
			long b = sub_part_2.get(i);
			long c = (a + b + retenue) % CVRCipher.alphabetSize;
			retenue = (a + b + retenue - c) / CVRCipher.alphabetSize;
			output.set(i + 1, c);
		}
		output.set(0, retenue);
	}
}
