## Postgres specific setup

### Data for tests
```sql
CREATE TABLE users (
  id              SERIAL PRIMARY KEY,
  firstname           VARCHAR(100) NOT NULL,
  lastname  VARCHAR(100) NULL,
  age INTEGER
);

INSERT INTO users (firstname, lastname, age) values('Jan', 'Jansen', 25);
INSERT INTO users (firstname, lastname, age) values('Max', 'Maxen', 28);
INSERT INTO users (firstname, lastname, age) values('Klaus', 'Klausen', 38);
INSERT INTO users (firstname, lastname, age) values('Jörg', 'Jansen', 60);
INSERT INTO users (firstname, lastname, age) values('James', 'Jansen', 25);
INSERT INTO users (firstname, lastname, age) values('Klaus', 'Jansen', 53);

database=# select * from users;
 id | firstname | lastname | age 
----+-----------+----------+-----
  1 | Jan       | Jansen   |  25
  2 | Max       | Maxen    |  28
  3 | Klaus     | Klausen  |  38
  4 | Jörg      | Jansen   |  60
  5 | James     | Jansen   |  25
  6 | Klaus     | Jansen   |  53
(6 rows)
```


## MySQL specific setup

MySQL restricts file imports and exports to one specific directory. However, the BasicEngine requirest hat each user can import/export tables from the home directory of the user. 

```sql
select * INTO outfile "/tmp/table" from users;
ERROR 1290 (HY000): The MySQL server is running with the --secure-file-priv option so it cannot execute this statement
``` 

```sql
mysql> SHOW VARIABLES LIKE "secure_file_priv";
+------------------+-----------------------+
| Variable_name    | Value                 |
+------------------+-----------------------+
| secure_file_priv | /var/lib/mysql-files/ |
+------------------+-----------------------+
```

To solve the problem the setting `secure_file_priv` has so be disabled.

```bash
echo 'secure_file_priv=""' >>  /etc/mysql/mysql.conf.d/mysqld.cnf
/etc/init.d/mysql restart
```


### Data for tests

```sql

CREATE TABLE users (
  id              int AUTO_INCREMENT,
  firstname           VARCHAR(100) NOT NULL,
  lastname  VARCHAR(100) NULL,
  age INTEGER,
  PRIMARY KEY (id)
);

INSERT INTO users (firstname, lastname, age) values('Jan', 'Jansen', 25);
INSERT INTO users (firstname, lastname, age) values('Max', 'Maxen', 28);
INSERT INTO users (firstname, lastname, age) values('Klaus', 'Klausen', 38);
INSERT INTO users (firstname, lastname, age) values('Jörg', 'Jansen', 60);
INSERT INTO users (firstname, lastname, age) values('James', 'Jansen', 25);
INSERT INTO users (firstname, lastname, age) values('Klaus', 'Jansen', 53);


mysql> select * from users;
+----+-----------+----------+------+
| id | firstname | lastname | age  |
+----+-----------+----------+------+
|  1 | Jan       | Jansen   |   25 |
|  2 | Max       | Maxen    |   28 |
|  3 | Klaus     | Klausen  |   38 |
|  4 | Jörg      | Jansen   |   60 |
|  5 | James     | Jansen   |   25 |
|  6 | Klaus     | Jansen   |   53 |
+----+-----------+----------+------+
6 rows in set (0.01 sec)
```
