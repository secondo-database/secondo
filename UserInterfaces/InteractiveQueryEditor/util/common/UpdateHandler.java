package util.common;

/**
 * Interface used as a part of an interceptor pattern.
 * Its purpose is the loose coupling between the class SecondoOutputReader
 * and ConsolePane
 * @author D.Merle
 */
public interface UpdateHandler {
	public void handleUpdate(String text);
}