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

Now the data directories can be created and initialized. In the following example, one MySQL master MySQL-installation (port 20101) and four worker MySQL-installations (port 20102 20103 20104 20105) are made:

```
mkdir -p /home/user/mysql/master
mkdir -p /home/user/mysql/worker1
mkdir -p /diskb/user/mysql/worker2
mkdir -p /diskc/user/mysql/worker3
mkdir -p /diskd/user/mysql/worker4

chown mysql.mysql /home/user/mysql/master
chown mysql.mysql /home/user/mysql/worker1
chown mysql.mysql /diskb/user/mysql/worker2
chown mysql.mysql /diskc/user/mysql/worker3
chown mysql.mysql /diskd/user/mysql/worker4

mysqld --initialize-insecure --user=mysql --datadir=/home/user/mysql/master
mysqld --initialize-insecure --user=mysql --datadir=/home/user/mysql/worker2
mysqld --initialize-insecure --user=mysql --datadir=/diskb/user/mysql/worker3
mysqld --initialize-insecure --user=mysql --datadir=/diskc/user/mysql/worker4
mysqld --initialize-insecure --user=mysql --datadir=/diskd/user/mysql/worker5
```

The MySQL installations can now be started by executing `mysqld_multi start`. The MySQL installations are now running. 


```bash
# Setup the user for mysql shutdown (see mysqld_multi setting above)
#
# Note: '...' has to be replaced by concrete values
#
export MYSQL_USER=...
export MYSQL_PASSWORD=...
export MYSQL_MASTER_DB...
export MYSQL_WORKER_DB=...

for i in 20101 20102 20103 20104 20105; do mysql -u root -h 127.0.0.1 -P $i -e "GRANT SHUTDOWN ON *.* TO 'mysql'@'localhost' IDENTIFIED BY 'secret';"; done

# Setup a user for the basic engine
# MySQL dinstingushes between users that are connecting over a pipe (host=LOCALHOST) and users that are connecting 
# over the network (HOST='%'). To be able to connect from both sources, the user has to created twice. 
for i in 20101 20102 20103 20104 20105; do mysql -u root -h 127.0.0.1 -P $i -e "CREATE USER '$MYSQL_USER'@'localhost' IDENTIFIED BY '$MYSQL_PASSWORD';"; done
for i in 20101 20102 20103 20104 20105; do mysql -u root -h 127.0.0.1 -P $i -e "CREATE USER '$MYSQL_USER'@'%' IDENTIFIED BY '$MYSQL_PASSWORD';"; done

# Grant the file permission to the user
for i in 20101 20102 20103 20104 20105; do mysql -u root -h 127.0.0.1 -P $i -e "GRANT FILE ON *.* TO '$MYSQL_USER'@'localhost';"; done
for i in 20101 20102 20103 20104 20105; do mysql -u root -h 127.0.0.1 -P $i -e "GRANT FILE ON *.* TO '$MYSQL_USER'@'%';"; done

# Create the masterdb
mysql -u root -h 127.0.0.1 -P 20101 -e "CREATE DATABASE $MYSQL_MASTER_DB CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;"

# Create the workerdb for the basic engine (UTF-8 encoding is required for importing shapefiles later)
for i in 20102 20103 20104 20105; do mysql -u root -h 127.0.0.1 -P $i -e "CREATE DATABASE $MYSQL_WORKER_DB CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;"; done

# Grant the permissions for the master database
mysql -u root -h 127.0.0.1 -P 20101 -e "GRANT ALL ON $MYSQL_MASTER_DB.* TO '$MYSQL_USER'@'localhost';"
mysql -u root -h 127.0.0.1 -P 20101 -e "GRANT ALL ON $MYSQL_MASTER_DB.* TO '$MYSQL_USER'@'%';"

# Grant the permissions for the worker database
for i in 20102 20103 20104 20105; do mysql -u root -h 127.0.0.1 -P $i -e "GRANT ALL ON $MYSQL_WORKER_DB.* TO '$MYSQL_USER'@'localhost';"; done
for i in 20102 20103 20104 20105; do mysql -u root -h 127.0.0.1 -P $i -e "GRANT ALL ON $MYSQL_WORKER_DB.* TO '$MYSQL_USER'@'%';"; done

# Set a root password ('secret' should be replaced by a real password)
for i in 20101 20102 20103 20104 20105; do mysql -u root -h 127.0.0.1 -P $i -e "ALTER USER 'root'@'localhost' IDENTIFIED BY 'secret';"; done
for i in 20101 20102 20103 20104 20105; do mysql -u root -h 127.0.0.1 -P $i -e "ALTER USER 'root'@'localhost' IDENTIFIED BY '%';"; done
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
     ("127.0.0.1" 50550 "SecondoConfig.ini" "user" "supersecret" 50507 "workerdb")
     ("127.0.0.1" 50551 "SecondoConfig.ini" "user" "supersecret" 50508 "workerdb")
     ("127.0.0.1" 50552 "SecondoConfig.ini" "user" "supersecret" 50509 "workerdb")
     ("127.0.0.1" 50553 "SecondoConfig.ini" "user" "supersecret" 50510 "workerdb")
     ("127.0.0.1" 50554 "SecondoConfig.ini" "user" "supersecret" 50511 "workerdb")
)]

query be_init_cluster('pgsql', 'user', 'supersecret', 50506, 'masterdb', WorkersPG)
```

### Connecting to MySQL
```
let WorkersMySQL = [const rel(tuple([Host: string, Port: int,
Config: string, DBUser: string, DBPass: string, DBPort: int, DBName: string])) value
(
     ("127.0.0.1" 50550 "SecondoConfig.ini" "user" "supersecret" 13301 "workerdb")
     ("127.0.0.1" 50551 "SecondoConfig.ini" "user" "supersecret" 13302 "workerdb")
     ("127.0.0.1" 50552 "SecondoConfig.ini" "user" "supersecret" 13303 "workerdb")
     ("127.0.0.1" 50553 "SecondoConfig.ini" "user" "supersecret" 13304 "workerdb")
     ("127.0.0.1" 50554 "SecondoConfig.ini" "user" "supersecret" 13305 "workerdb")
)]

query be_init_cluster('mysql', 'user', 'supersecret', 13300, 'masterdb', WorkersMySQL)
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

query be_partGrid(’roads’, ’gid’, ’geog’, 5.8, 50.3, 0.2, 20);
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

```sql
# Build example grid
CALL ST_CREATE_STATIC_GRID(100, 100, 0.5, 0.5, 0, 0); 

# List grid in MySQL
SELECT id, ST_AsWKT(cell), ST_SRID(cell) FROM grid_table;
```

```
# Partition spatial data (postgres)
query be_grid_creaet('mygrid', 5.8, 50.3, 0.2, 20, 20);
query be_part_grid('water','gid', 'geog', 'mygrid', 20);
query be_grid_delete('mygrid);

# Partition spatial data (MySQL)
query be_grid_create('mygrid', 5.8, 50.3, 0.2, 20, 20);
query be_part_grid('water', 'OGR_FID', 'SHAPE', 'mygrid', 20);
query be_grid_delete('mygrid);
```

## Working with spatial data

In this section, the handling of spatial data with the BasicEngine is shown.

### Import OSM Data (Postgres)

```
# Ensure Postgis is installed (contains shp2pgsql)
apt-get install postgis

# Download data
mkdir nrw_data
cd nrw_data
wget http://download.geofabrik.de/europe/germany/nordrhein-westfalen-latest-free.shp.zip
unzip nordrhein-westfalen-latest-free.shp.zip 

# Convert to Postgres 
# (-s = projection, -d = drop table -G = use the geography type)
# 4326 is WGS 84
shp2pgsql -s 4326 -d -G gis_osm_buildings_a_free_1 public.buildings > buildings.sql
shp2pgsql -s 4326 -d -G gis_osm_landuse_a_free_1 public.landuse > landuse.sql
shp2pgsql -s 4326 -d -G gis_osm_natural_free_1 public.natural > natural.sql
shp2pgsql -s 4326 -d -G gis_osm_places_free_1 public.places > places.sql
shp2pgsql -s 4326 -d -G gis_osm_pofw_a_free_1 public.pofw > pofw.sql
shp2pgsql -s 4326 -d -G gis_osm_pois_free_1 public.pois > pois.sql
shp2pgsql -s 4326 -d -G gis_osm_railways_free_1 public.railway > railway.sql
shp2pgsql -s 4326 -d -G gis_osm_roads_free_1 public.roads > roads.sql
shp2pgsql -s 4326 -d -G gis_osm_traffic_free_1 public.traffic > traffic.sql
shp2pgsql -s 4326 -d -G gis_osm_transport_free_1 public.transport > transport.sql
shp2pgsql -s 4326 -d -G gis_osm_water_a_free_1 public.water > water.sql
shp2pgsql -s 4326 -d -G gis_osm_waterways_free_1 public.waterways > waterways.sql

# Import SQL into Postgres
for i in *.sql; do 
  echo "Importing $i"
  psql -h localhost -p 5432 -U <USER> -d <DATABASE> -f $i
done
```


### Import OSM Data (MySQL)

Shapefiles are loaded into MySQL using [ogr2ogr](https://gdal.org/drivers/vector/mysql.html).

```
# Ensure gdal-bin is installed (contains ogr2ogr and ogrinfo)
apt-get install gdal-bin

# Download data
mkdir nrw_data
cd nrw_data
wget http://download.geofabrik.de/europe/germany/nordrhein-westfalen-latest-free.shp.zip
unzip nordrhein-westfalen-latest-free.shp.zip 

export MYSQL_USER=...
export MYSQL_PASS=...
export MYSQL_DB=masterdb
export MYSQL_MASTER_PORT=20101

# Import shapefiles into MySQL
# (-nln layername)
# (-skipfailures to ignore some UTF-8 encoding errors)
#
# Use the MyISAM storage engine for performance reasons. 
# ogr2ogr does not encapsulate the inserts in bulk transactions. 
# Therefore, one transaction per insert would be performed when 
# the INNODB storage engine is used, which leads to poor import 
# performance. 
#
# 'natural' is a reserved keyword in SQL. Therefore, the natural data
# is imported into the table natural_data
# 
ogr2ogr -f MySQL MySQL:$MYSQL_DB,user=$MYSQL_USER,password=$MYSQL_PASS,port=$MYSQL_MASTER_PORT,host=127.0.0.1 gis_osm_buildings_a_free_1.shp -nln buildings -update -overwrite -lco engine=MyISAM -skipfailures
ogr2ogr -f MySQL MySQL:$MYSQL_DB,user=$MYSQL_USER,password=$MYSQL_PASS,port=$MYSQL_MASTER_PORT,host=127.0.0.1 gis_osm_landuse_a_free_1.shp -nln landuse -update -overwrite -lco engine=MyISAM -skipfailures
ogr2ogr -f MySQL MySQL:$MYSQL_DB,user=$MYSQL_USER,password=$MYSQL_PASS,port=$MYSQL_MASTER_PORT,host=127.0.0.1 gis_osm_natural_free_1.shp -nln natural_data -update -overwrite -lco engine=MyISAM -skipfailures
ogr2ogr -f MySQL MySQL:$MYSQL_DB,user=$MYSQL_USER,password=$MYSQL_PASS,port=$MYSQL_MASTER_PORT,host=127.0.0.1 gis_osm_places_free_1.shp -nln places -update -overwrite -lco engine=MyISAM -skipfailures
ogr2ogr -f MySQL MySQL:$MYSQL_DB,user=$MYSQL_USER,password=$MYSQL_PASS,port=$MYSQL_MASTER_PORT,host=127.0.0.1 gis_osm_pofw_a_free_1.shp -nln pofw -update -overwrite -lco engine=MyISAM -skipfailures
ogr2ogr -f MySQL MySQL:$MYSQL_DB,user=$MYSQL_USER,password=$MYSQL_PASS,port=$MYSQL_MASTER_PORT,host=127.0.0.1 gis_osm_pois_free_1.shp -nln pois -update -overwrite -lco engine=MyISAM -skipfailures
ogr2ogr -f MySQL MySQL:$MYSQL_DB,user=$MYSQL_USER,password=$MYSQL_PASS,port=$MYSQL_MASTER_PORT,host=127.0.0.1 gis_osm_railways_free_1.shp -nln railways -update -overwrite -lco engine=MyISAM -skipfailures
ogr2ogr -f MySQL MySQL:$MYSQL_DB,user=$MYSQL_USER,password=$MYSQL_PASS,port=$MYSQL_MASTER_PORT,host=127.0.0.1 gis_osm_roads_free_1.shp -nln roads -update -overwrite -lco engine=MyISAM -skipfailures
ogr2ogr -f MySQL MySQL:$MYSQL_DB,user=$MYSQL_USER,password=$MYSQL_PASS,port=$MYSQL_MASTER_PORT,host=127.0.0.1 gis_osm_traffic_free_1.shp -nln traffic -update -overwrite -lco engine=MyISAM -skipfailures
ogr2ogr -f MySQL MySQL:$MYSQL_DB,user=$MYSQL_USER,password=$MYSQL_PASS,port=$MYSQL_MASTER_PORT,host=127.0.0.1 gis_osm_transport_free_1.shp -nln transport -update -overwrite -lco engine=MyISAM -skipfailures
ogr2ogr -f MySQL MySQL:$MYSQL_DB,user=$MYSQL_USER,password=$MYSQL_PASS,port=$MYSQL_MASTER_PORT,host=127.0.0.1 gis_osm_water_a_free_1.shp -nln water -update -overwrite -lco engine=MyISAM -skipfailures
ogr2ogr -f MySQL MySQL:$MYSQL_DB,user=$MYSQL_USER,password=$MYSQL_PASS,port=$MYSQL_MASTER_PORT,host=127.0.0.1 gis_osm_waterways_free_1.shp -nln waterways -update -overwrite -lco engine=MyISAM -skipfailures

# Change SRID to 4326 (WGS84)
for i in buildings landuse natural_data places pofw pois railways roads traffic transport water waterways; do
   echo "Fixig the spatial reference identifier (SRID) of the spatial data in table $i"
   mysql -u $MYSQL_USER -p$MYSQL_PASS --port $MYSQL_MASTER_PORT -h 127.0.0.1 $MYSQL_DB -e "UPDATE $i SET SHAPE = ST_GeomFromText(ST_AsText(SHAPE), 4326);"
done

# Change storage backend in MySQL to InnoDB
for i in buildings landuse natural_data places pofw pois railways roads traffic transport water waterways; do
   echo "Converting table $i to INNODB storage engine"
   mysql -u $MYSQL_USER -p$MYSQL_PASS --port $MYSQL_MASTER_PORT -h 127.0.0.1 $MYSQL_DB -e "ALTER TABLE $i ENGINE=InnoDB;"
done
``` 

# Retrieve summary information
```
ogrinfo MySQL:$MYSQL_DB,user=$MYSQL_USER,password=$MYSQL_PASS,port=$MYSQL_MASTER_PORT,host=127.0.0.1 buildings -so

Layer name: buildings
Geometry: Polygon
Feature Count: 116182
Extent: (5.866689, 50.363445) - (9.409564, 52.531316)
Layer SRS WKT:
GEOGCS["GCS_WGS_1984",
    DATUM["WGS_1984",
        SPHEROID["WGS_84",6378137,298.257223563]],
    PRIMEM["Greenwich",0],
    UNIT["Degree",0.017453292519943295],
    AUTHORITY["EPSG","4326"]]
FID Column = OGR_FID
Geometry Column NOT NULL = 
osm_id: String (10.0)
code: Real (4.0)
fclass: String (28.0)
name: String (100.0)
type: String (20.0)


# Fetch information about the SIRD 4326
SELECT * FROM INFORMATION_SCHEMA.ST_SPATIAL_REFERENCE_SYSTEMS WHERE SRS_ID = 4326\G

# Get the SRID of the spatial data
SELECT ST_SRID(SHAPE) FROM waterways LIMIT 1;
```

# Import and Export MySQL relations with Shape datatypes
```sql
-- Regular import and export of relations with SHAPE datatype fails
SELECT * FROM water INTO OUTFILE "/tmp/water" CHARACTER SET utf8mb4;
CREATE TABLE water_import  SELECT * FROM water limit 0;
LOAD DATA INFILE "/tmp/water" into table water_import CHARACTER SET utf8;
ERROR 1416 (22003): Cannot get geometry object from data you send to the GEOMETRY field

-- Datatype has to be serialized manually
SELECT OGR_FID,ST_AsWKT(SHAPE),ST_SRID(SHAPE),osm_id,code,fclass,name FROM water INTO OUTFILE "/tmp/water" CHARACTER SET utf8mb4;
CREATE TABLE water_import  SELECT * FROM water limit 0;
LOAD DATA INFILE "/tmp/water" into table water_import CHARACTER SET utf8 (@col1, @col2, @col3, @col4, @col5, @col6, @col7) SET `OGR_FID` = @col1, SHAPE = ST_PolygonFromText(@col2, @col3), osm_id = @col4, code = @col5, fclass = @col6, name = @col7;
```