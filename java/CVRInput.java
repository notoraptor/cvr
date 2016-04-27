package liber.security.cvr;

import java.io.Closeable;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;

// Déchiffrement.
public class CVRInput implements Closeable {
	private FileInputStream fileInputStream;
	private CVRPasswords cvrPasswords;
	private CVRPassword cvrPassword;
	private int caractere;
	private int caracterePrecedent;
	private int j;
	private long sommeQuotients;
	// On admet que la clé peut contenir des caractères UTF-8.
	public CVRInput(File inputFile, String key) throws Exception {
		fileInputStream = new FileInputStream(inputFile);
		cvrPasswords = new CVRPasswords(new UnsignedBytes(key.getBytes(StandardCharsets.UTF_8)));
		cvrPassword = cvrPasswords.currentPassword();
	}
	@Override
	public void close() throws IOException {
		fileInputStream.close();
	}
	public int read() throws Exception {
		int output = -1;
		int caractere = fileInputStream.read();
		if (caractere != -1) {
			if (cvrPassword.isEmpty())
				cvrPassword = cvrPasswords.next();
			j = (j + caracterePrecedent) % cvrPassword.length();
			long c = caractere;
			long u = cvrPassword.nextCharacter();
			long v = cvrPassword.character(j);
			if (u == 0) {
				u = sommeQuotients % CVRCipher.alphabetSize;
				sommeQuotients = sommeQuotients / CVRCipher.alphabetSize;
			}
			int q = ((CVRCipher.alphabetSize - 1 - CVRCipher.toDecrypt(c, u, v)) / CVRCipher.alphabetSize);
			int t = CVRCipher.alphabetSize * q + CVRCipher.toDecrypt(c, u, v);
			sommeQuotients += q;
			output = t;
			caracterePrecedent = t;
		}
		return output;
	}
}
