package twodsack.util.collection;

public class ComparatorNotDefinedException extends RuntimeException {
    ComparatorNotDefinedException() { super(); }
    ComparatorNotDefinedException(String s) { super(s); }
}