package liber.security.cvr;

import java.util.Arrays;

class UnsignedBytes {
	private byte[] bytes;
	private int start;
	public UnsignedBytes(byte[] theBytes) {
		assert theBytes != null;
		bytes = theBytes;
	}
	public UnsignedBytes(int taille) {
		bytes = new byte[taille];
	}
	public UnsignedBytes(UnsignedBytes src, int from) {
		bytes = src.bytes;
		start = from;
	}
	public int size() {
		return bytes.length;
	}
	public int get(int position) {
		return bytes[start + position] & 0xff;
	}
	public void set(int position, int unsignedByte) {
		bytes[start + position] = (byte) unsignedByte;
	}
	public void set(int position, long unsignedByte) {
		bytes[start + position] = (byte) unsignedByte;
	}
	public void memclear(int length) {
		Arrays.fill(bytes, start, start + length, (byte) 0);
	}
	public void copy(int from, UnsignedBytes src, int srcLength) {
		System.arraycopy(src.bytes, src.start, bytes, start + from, srcLength);
	}
	@Override
	public String toString() {
		StringBuilder s = new StringBuilder();
		if(start != 0)
			s.append('[').append(start).append('/').append(bytes.length).append("] ");
		for(int i = start; i < bytes.length; ++i) {
			s.append('(').append((bytes[i] & 0xff)).append(')');
		}
		return s.toString();
	}
}
