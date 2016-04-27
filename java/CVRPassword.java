package liber.security.cvr;

class CVRPassword {
	private UnsignedBytes key;
	private int keyLength;
	private int index;
	public void define(UnsignedBytes chaine, int depart, int taille) {
		assert chaine != null;
		key = new UnsignedBytes(chaine, depart);
		keyLength = taille;
		index = 0;
	}
	int nextCharacter() throws Exception {
		if (index == keyLength)
			throw new Exception("Mot de passe vide.");
		return key.get(index++);
	}
	int character(int position) throws Exception {
		if (position >= keyLength)
			throw new Exception("Position hors des limites dans le mot de passe actuel.");
		return key.get(position);
	}
	int length() {
		return keyLength;
	}
	boolean isEmpty() {
		return index == keyLength;
	}
	@Override
	public String toString() {
		return key.toString();
	}
}
