package gui;

public interface SecondoChangeListener{

 // deleted or created database
 void databasesChanged();
 // deleted or created or updated object
 void objectsChanged();
 // deleted or create type
 void typesChanged();
 // a database is opened
 void databaseOpened();
 // a database is closed
 void databaseClosed();


}