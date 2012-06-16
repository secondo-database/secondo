package LocalTest;

import java.io.*;

public class TestReporter {
	
	private static FileWriter f1;
	
	
	public static void startFileWriting(String path) {
		try {
			f1 = new FileWriter(path);
		}
		catch(IOException e) {
			System.out.println("Error in creating file");
		}
	}
	
	public static void lnforward() {
		try {
			f1.write("\n\r\n\r");
		}
		catch(IOException e) {
			System.out.println("Error in writing into file");
		}
	}
	
	public static void endFileWriting() {
		try {
			f1.close();
		}
		catch(IOException e) {
			System.out.println("Error in closing file");
		}
	}
	
	public static void writeInFile(String text) {
		try {
			f1.write(text+"\n\r\n\r");
		}
		catch(Exception e) {
			System.out.println("Error in writing into file");
		}
	}
}
