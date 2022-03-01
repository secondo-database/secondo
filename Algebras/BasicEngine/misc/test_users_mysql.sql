CREATE TABLE users (
  id        int AUTO_INCREMENT,
  firstname VARCHAR(100) NOT NULL,
  lastname  VARCHAR(100) NULL,
  age       INTEGER,
  PRIMARY KEY (id)
);

INSERT INTO users (firstname, lastname, age) values('Jan', 'Jansen', 25);
INSERT INTO users (firstname, lastname, age) values('Max', 'Maxen', 28);
INSERT INTO users (firstname, lastname, age) values('Klaus', 'Klausen', 38);
INSERT INTO users (firstname, lastname, age) values('Jörg', 'Jansen', 60);
INSERT INTO users (firstname, lastname, age) values('James', 'Jansen', 25);
INSERT INTO users (firstname, lastname, age) values('Klaus', 'Jansen', 53);

