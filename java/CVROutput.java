package liber.security.cvr;

import java.io.Closeable;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;

// Chiffrement.
public class CVROutput implements Closeable {
	static private final String lineSeparator = System.getProperty("line.separator");
	private FileOutputStream fileOutputStream;
	private CVRPasswords cvrPasswords;
	private CVRPassword cvrPassword;
	private int caracterePrecedent;
	private int j;
	private long sommeQuotients;
	// On admet que la clé peut contenir des caractères UTF-8.
	public CVROutput(File outputFile, String key) throws Exception {
		fileOutputStream = new FileOutputStream(outputFile);
		cvrPasswords = new CVRPasswords(new UnsignedBytes(key.getBytes(StandardCharsets.UTF_8)));
		cvrPassword = cvrPasswords.currentPassword();
	}
	@Override
	public void close() throws IOException {
		fileOutputStream.close();
	}
	// Chiffrement
	public void write(int character) throws Exception {
		if (cvrPassword.isEmpty())
			cvrPassword = cvrPasswords.next();
		j = (j + caracterePrecedent) % cvrPassword.length();
		long t = character;
		long u = cvrPassword.nextCharacter();
		long v = cvrPassword.character(j);
		if (u == 0) {
			u = sommeQuotients % CVRCipher.alphabetSize;
			sommeQuotients = sommeQuotients / CVRCipher.alphabetSize;
		}
		int c = (int) (CVRCipher.toEncrypt(t, u, v) % CVRCipher.alphabetSize);
		sommeQuotients += CVRCipher.toEncrypt(t, u, v) / CVRCipher.alphabetSize;
		fileOutputStream.write(c);
		caracterePrecedent = character;
	}
	// On admet que la chaîne ne contient que des caractères ASCII.
	public void write(String string) throws Exception {
		for (byte character : string.getBytes())
			write(character);
	}
	public void newLine() throws Exception {
		write(lineSeparator);
	}
}
