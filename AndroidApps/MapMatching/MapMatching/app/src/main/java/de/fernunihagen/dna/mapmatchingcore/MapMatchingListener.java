package de.fernunihagen.dna.mapmatchingcore;

public interface MapMatchingListener {
    public abstract void showLineResultMessage(String message);
    public abstract void showResultMessage(String message);
    public abstract void showLineInfoMessage(String message);
    public abstract void showInfoMessage(String message);
    public abstract void showLineErrorMessage(String message);
    public abstract void showErrorMessage(String message);
    public abstract void reset();
}
