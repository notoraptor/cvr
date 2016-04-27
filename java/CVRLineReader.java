package liber.security.cvr;

public class CVRLineReader {
	private CVRInput cvrInput;
	private int character;
	public CVRLineReader(CVRInput cvrInput) {
		this.cvrInput = cvrInput;
	}
	public String readLine() throws Exception {
		StringBuilder builder = null;
		if (character != -1) {
			while ((character = cvrInput.read()) != -1) {
				char c = (char) character;
				if (c != '\r' && c != '\n')
					break;
			}
			if (character != -1) {
				builder = new StringBuilder();
				builder.append((char) character);
				while ((character = cvrInput.read()) != -1) {
					char c = (char) character;
					if (c == '\r' || c == '\n')
						break;
					else
						builder.append(c);
				}
			}
		}
		return builder == null ? null : builder.toString();
	}
}
