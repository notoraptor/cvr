package liber.security.cvr;

class CVRPasswords {
	private int term;    // indice n du terme (U[n]) de la suite de mots de passe générés.
	private int m;
	private UnsignedBytes[] list;
	private UnsignedBytes current;
	private CVRPassword cvrPassword;
	public CVRPasswords(UnsignedBytes cle) throws Exception {
		assert cle != null && cle.size() != 0;
		m = cle.size();
		list = new UnsignedBytes[11];
		for (int i = 0; i < 11; ++i)
			list[i] = new UnsignedBytes(m + 17);
		cvrPassword = new CVRPassword();
		CVRSensibility cvrSensibility = new CVRSensibility(m + 16);
		cvrSensibility.define(cle, m);
		cvrSensibility.compute();
		list[0].copy(17, cle, m);
		list[1].copy(0, cvrSensibility.current(), m + 17);
		for (int i = 2; i < 10; ++i) {
			cvrSensibility.add(list[i - 2], m + 17);
			cvrSensibility.compute();
			list[i].copy(0, cvrSensibility.current(), m + 17);
		}
		current = new UnsignedBytes(m);
		term = 9;
	}
	public CVRPassword next() {
		int taille;
		int entree;
		do {
			++term;
			entree = term % 11;
			list[entree].copy(0, list[(term - 10) % 11], m + 17);
			CVRCipher.sumOnSite(list[entree], list[(term - 7) % 11], m + 17, m + 17);
			list[entree].memclear(17);
			int vide;
			for (vide = 0; vide < m && list[entree].get(vide + 17) == 0; ++vide) ;
			taille = m - vide;
		} while (taille == 0);
		current.copy((m - taille), new UnsignedBytes(list[entree], (m + 17 - taille)), taille);
		cvrPassword.define(current, m - taille, taille);
		return cvrPassword;
	}
	public CVRPassword currentPassword() {
		return cvrPassword;
	}
}
