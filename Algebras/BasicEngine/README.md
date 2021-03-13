## Postgres specific setup

```bash
# Create a master postgres and 5 worker on different disks and ports
sudo pg_createcluster 10 user_master -p 50506 
sudo pg_createcluster 10 user_worker1 -p 50507 -d /home/user/postgres/worker1
sudo pg_createcluster 10 user_worker2 -p 50508 -d /home/user/postgres/worker2
sudo pg_createcluster 10 user_worker3 -p 50509 -d /diskb/user/postgres/worker3
sudo pg_createcluster 10 user_worker4 -p 50510 -d /diskc/user/postgres/worker4
sudo pg_createcluster 10 user_worker5 -p 50511 -d /diskd/user/postgres/worker5

# Start the postgres installations
sudo pg_ctlcluster 10 user_master start 
sudo pg_ctlcluster 10 user_worker1 start 
sudo pg_ctlcluster 10 user_worker2 start 
sudo pg_ctlcluster 10 user_worker3 start 
sudo pg_ctlcluster 10 user_worker4 start 
sudo pg_ctlcluster 10 user_worker5 start 

# Setup a user 'username' with password 'supersecret' (adjust the ports to your setup)
for i in 50506 50507 50508 50509 50510 50511; do sudo su - postgres -c "psql -p $i -c 'CREATE ROLE username LOGIN SUPERUSER PASSWORD 'supersecret';"; done

# Create a database named database
for i in 50506 50507 50508 50509 50510 50511; do sudo su - postgres -c "psql -p $i -c 'CREATE DATABASE database;'"; done

# List active postgres processes
pg_lsclusters
```

```bash
# Perform a SQL query on all local postgres worker installations
for i in 50507 50508 50509 50510 50511; do sudo su - postgres -c "psql -p $i -d database -c 'SELECT * FROM users'"; done
```

```bash
# Deinstallation
sudo pg_dropcluster  --stop 10 user_master
sudo pg_dropcluster  --stop 10 user_worker1 
sudo pg_dropcluster  --stop 10 user_worker2
sudo pg_dropcluster  --stop 10 user_worker3
sudo pg_dropcluster  --stop 10 user_worker4
sudo pg_dropcluster  --stop 10 user_worker5
```

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

In addition, AppArmor can prevent MySQL from reading/writing files into the home directory of the users. This can be solved by adding additional rules to the AppArmour configuration (`/etc/apparmor.d/usr.sbin.mysqld`).

```
# Allow import / export of the SECONDO basic engine
  @{HOME}/filetransfer/ r,
  @{HOME}/filetransfer/** rwk,

# Additional SECONDO database dirs should be also covered
#
#  /diskb/ r,
#  /diskb/** rwk,
```

After the configuration is changed, the changed AppArmour profile needs to be applied ( `sudo apparmor_parser -r /etc/apparmor.d/usr.sbin.mysqld`) and MySQL needs to be restarted (`sudo service mysql restart`). 

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
