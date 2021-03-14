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

To utilize multiple disks in one system, `mysqld_multi` can be used to start multiple MySQL-Servers with different data directories. Add the following lines to your `/etc/mysql/mysql.cnf`:


```
[mysqld_multi]
mysqld        = /usr/bin/mysqld_safe
mysqladmin    = /usr/bin/mysqladmin
log           = /var/log/mysql/mysqld_multi.log
user          = mysql
pass          = secret

[mysqld1]
user                    = mysql
pid-file                = /var/run/mysqld/mysqld1.pid
socket                  = /var/run/mysqld/mysqld1.sock
basedir                 = /usr
datadir                 = /home/user/mysql/worker1
tmpdir                  = /tmp
lc-messages-dir         = /usr/share/mysql
bind-address            = 127.0.0.1
query_cache_size        = 16M
log_error               = /var/log/mysql/mysqld1_error.log
expire_logs_days        = 10
character-set-server    = utf8mb4
collation-server        = utf8mb4_general_ci
secure_file_priv        = ""
port                    = 20101

[mysqld2]
user                    = mysql
pid-file                = /var/run/mysqld/mysqld2.pid
socket                  = /var/run/mysqld/mysqld2.sock
basedir                 = /usr
datadir                 = /home/user/mysql/worker2
tmpdir                  = /tmp
lc-messages-dir         = /usr/share/mysql
bind-address            = 127.0.0.1
query_cache_size        = 16M
log_error               = /var/log/mysql/mysqld2_error.log
expire_logs_days        = 10
character-set-server    = utf8mb4
collation-server        = utf8mb4_general_ci
secure_file_priv        = ""
port                    = 20102

[mysqld3]
user                    = mysql
pid-file                = /var/run/mysqld/mysqld3.pid
socket                  = /var/run/mysqld/mysqld3.sock
basedir                 = /usr
datadir                 = /diskb/user/mysql/worker3
tmpdir                  = /tmp
lc-messages-dir         = /usr/share/mysql
bind-address            = 127.0.0.1
query_cache_size        = 16M
log_error               = /var/log/mysql/mysqld3_error.log
expire_logs_days        = 10
character-set-server    = utf8mb4
collation-server        = utf8mb4_general_ci
secure_file_priv        = ""
port                    = 20103

[mysqld4]
user                    = mysql
pid-file                = /var/run/mysqld/mysqld4.pid
socket                  = /var/run/mysqld/mysqld4.sock
basedir                 = /usr
datadir                 = /diskc/user/mysql/worker4
tmpdir                  = /tmp
lc-messages-dir         = /usr/share/mysql
bind-address            = 127.0.0.1
query_cache_size        = 16M
log_error               = /var/log/mysql/mysqld4_error.log
expire_logs_days        = 10
character-set-server    = utf8mb4
collation-server        = utf8mb4_general_ci
secure_file_priv        = ""
port                    = 20104

[mysqld5]
user                    = mysql
pid-file                = /var/run/mysqld/mysqld5.pid
socket                  = /var/run/mysqld/mysqld5.sock
basedir                 = /usr
datadir                 = /diskd/user/mysql/worker5
tmpdir                  = /tmp
lc-messages-dir         = /usr/share/mysql
bind-address            = 127.0.0.1
query_cache_size        = 16M
log_error               = /var/log/mysql/mysqld5_error.log
expire_logs_days        = 10
character-set-server    = utf8mb4
collation-server        = utf8mb4_general_ci
secure_file_priv        = ""
port                    = 20105
```

You can list all your MySQL-Multi installations by calling ` mysqld_multi report`. 

If you are using AppArmour, add the following lines to your MySQL-AppArmour configuration `/etc/apparmor.d/usr.sbin.mysqld`:

```
  /var/run/mysqld/ r,
  /var/run/mysqld/** rwk,
  /run/mysqld/ r,
  /run/mysqld/** rwk,
```

and calling `apparmor_parser -r /etc/apparmor.d/usr.sbin.mysqld`.

Now the data directories can be created and initialized:

```
mkdir -p /home/user/mysql/worker1
mkdir -p /home/user/mysql/worker2
mkdir -p /diskb/user/mysql/worker3
mkdir -p /diskc/user/mysql/worker4
mkdir -p /diskd/user/mysql/worker5

chown mysql.mysql /home/user/mysql/worker1
chown mysql.mysql /home/user/mysql/worker2
chown mysql.mysql /diskb/user/mysql/worker3
chown mysql.mysql /diskc/user/mysql/worker4
chown mysql.mysql /diskd/user/mysql/worker5

mysqld --initialize-insecure --user=mysql --datadir=/home/user/mysql/worker1
mysqld --initialize-insecure --user=mysql --datadir=/home/user/mysql/worker2
mysqld --initialize-insecure --user=mysql --datadir=/diskb/user/mysql/worker3
mysqld --initialize-insecure --user=mysql --datadir=/diskc/user/mysql/worker4
mysqld --initialize-insecure --user=mysql --datadir=/diskd/user/mysql/worker5
```

The MySQL installations can now be started by executing `mysqld_multi start`. The MySQL installations are now running. 


```bash
# Setup the user for mysql shutdown (see mysqld_multi setting above)
for i in 20101 20102 20103 20104 20105; do mysql -u root -h 127.0.0.1 -P $i -e "GRANT SHUTDOWN ON *.* TO 'mysql'@'localhost' IDENTIFIED BY 'secret';"; done

# Setup a user for the basic engine
for i in 20101 20102 20103 20104 20105; do mysql -u root -h 127.0.0.1 -P $i -e "CREATE USER 'user'@'localhost' IDENTIFIED BY '123geheim456';"; done

# Grant the file permission to the user
for i in 20101 20102 20103 20104 20105; do mysql -u root -h 127.0.0.1 -P $i -e "GRANT FILE ON *.* TO 'user'@'localhost';"; done

# Create the workerdb for the basic engine
for i in 20101 20102 20103 20104 20105; do mysql -u root -h 127.0.0.1 -P $i -e "CREATE DATABASE workerdb;"; done

# Grant the permissions for the database
for i in 20101 20102 20103 20104 20105; do mysql -u root -h 127.0.0.1 -P $i -e "GRANT ALL ON workerdb.* TO 'user'@'localhost';"; done

# Set a root password
for i in 20101 20102 20103 20104 20105; do mysql -u root -h 127.0.0.1 -P $i -e "ALTER USER 'root'@'localhost' IDENTIFIED BY 'secret';"; done
```

### MySQL settings

MySQL restricts file imports and exports to one specific directory. However, the BasicEngine requires that each user can import/export tables from the user's home directory. 

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

To solve the problem, the setting `secure_file_priv` has to be disabled.

```bash
echo 'secure_file_priv=""' >>  /etc/mysql/mysql.conf.d/mysqld.cnf
/etc/init.d/mysql restart
```

In addition, AppArmor can prevent MySQL from reading/writing files into the home directory of the users. This can be solved by adding additional rules to the AppArmour configuration (`/etc/apparmor.d/usr.sbin.mysqld`).

```
# Allow import / export of the SECONDO basic engine
  @{HOME}/filetransfer/ r,
  @{HOME}/filetransfer/** rwk,

# Additional SECONDO database directories should also be covered
#
#  /diskb/ r,
#  /diskb/** rwk,
```

After the configuration is changed, the changed AppArmour profile needs to be applied ( `sudo apparmor_parser -r /etc/apparmor.d/usr.sbin.mysqld`) and MySQL needs to be restarted (`sudo service mysql restart`). 

### Data for tests

```sql

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


## BasicEngine

### Connecting to Postgres
```
let WorkersPG = [const rel(tuple([Host: string, Port: int,
Config: string, DBUser: string, DBPass: string, DBPort: int, DBName: string])) value
(
     ("127.0.0.1" 50550 "SecondoConfig.ini" "user" "supersecret" 50507 "database")
     ("127.0.0.1" 50551 "SecondoConfig.ini" "user" "supersecret" 50508 "database")
     ("127.0.0.1" 50552 "SecondoConfig.ini" "user" "supersecret" 50509 "database")
     ("127.0.0.1" 50553 "SecondoConfig.ini" "user" "supersecret" 50510 "database")
     ("127.0.0.1" 50554 "SecondoConfig.ini" "user" "supersecret" 50511 "database")
)]

query be_init_cluster('pgsql', 'user', 'supersecret', 50506, 'database', WorkersPG)
```

### Connecting to MySQL
```
let WorkersMySQL = [const rel(tuple([Host: string, Port: int,
Config: string, DBUser: string, DBPass: string, DBPort: int, DBName: string])) value
(
     ("127.0.0.1" 50550 "SecondoConfig.ini" "user" "supersecret" 13301 "database")
     ("127.0.0.1" 50551 "SecondoConfig.ini" "user" "supersecret" 13302 "database")
     ("127.0.0.1" 50552 "SecondoConfig.ini" "user" "supersecret" 13303 "database")
     ("127.0.0.1" 50553 "SecondoConfig.ini" "user" "supersecret" 13304 "database")
     ("127.0.0.1" 50554 "SecondoConfig.ini" "user" "supersecret" 13305 "database")
)]

query be_init_cluster('mysql', 'user', 'supersecret', 13300, 'database', WorkersMySQL)
```

### Demo queries
```
# Partition relation roads, count elements and reduce result
query be_mcommand('CREATE EXTENSION postgis')
query be_command('CREATE EXTENSION postgis')

query be_mcommand("DROP TABLE roads")
query be_mcommand("DROP TABLE roads_count")
query be_part_hash("roads", "osm_id", 60)
query be_mquery('SELECT count(*) FROM roads', 'roads_count')
query be_union('roads_count');

query be_collect('SELECT * FROM roads_count') consume
query be_collect('SELECT * FROM roads_count') sum[Count]
query be_collect('SELECT count(*) FROM roads') consume
```

```
# Share table with all worker
query be_share('users');
```

```
# Validate the given query
query be_validate_query("SELECT * FROM users")
```

```
# Partition and re-partition releation roads
query be_part_hash("users", "firstname", 60)

query be_mcommand("DROP TABLE users")

query be_repart_hash("users", "firstname", 60)
query be_repart_hash("users", "lastname", 60)
query be_repart_hash("users", "age", 60)
```

