package liber.security.cvr;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;

public class CVR {
	static private final String extension = ".cvr";
	static public boolean isCVRFile(String filename, StringBuilder outname) {
		boolean test = false;
		if (filename.toLowerCase().endsWith(extension)) {
			test = true;
			outname.append(filename.substring(0, filename.length() - extension.length()));
		} else {
			outname.append(filename).append(extension);
		}
		return test;
	}
	static public void main(String[] args) throws Exception {
		if (args.length == 2) {
			String path = args[0];
			String key = args[1];
			File filePath = new File(path);
			StringBuilder outFilename = new StringBuilder();
			try {
				if(isCVRFile(path, outFilename)) {
					System.err.println("Fichier a dechiffrer.");
					try (
						CVRInput cvrInput = new CVRInput(filePath, key);
						FileOutputStream fileOutputStream = new FileOutputStream(new File(outFilename.toString()))
					) {
						int character;
						while ((character = cvrInput.read()) != -1)
							fileOutputStream.write(character);
					}
				} else {
					System.err.println("Fichier a chiffrer.");
					System.err.println(outFilename);
					try (
						FileInputStream fileInputStream = new FileInputStream(filePath);
						CVROutput cvrOutput = new CVROutput(new File(outFilename.toString()), key)
					) {
						int character;
						while ((character = fileInputStream.read()) != -1) {
							cvrOutput.write(character);
						}
					}
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
		} else {
			System.out.println("Utilisation :");
			System.out.println("\tPour chiffrer :");
			System.out.println("\t\tjava CVR <fichier-non-cvr> <mot-de-passe>");
			System.out.println("\tPour dechiffrer :");
			System.out.println("\t\tjava CVR <fichier-cvr> <mot-de-passe>");
			System.out.println("\tUn fichier CVR est un fichier qui a l'extension \".cvr\". " +
					"il s'agit normalement d'un fichier chiffre par ce programme.");
		}
	}
}
