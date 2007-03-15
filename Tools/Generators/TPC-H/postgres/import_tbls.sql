-- import tpch relations

\copy customer FROM 's05pp/customer.tbl.pg' WITH DELIMITER AS '|'
\copy lineitem FROM 's05pp/lineitem.tbl.pg' WITH DELIMITER AS '|'
\copy orders FROM 's05pp/orders.tbl.pg' WITH DELIMITER AS '|'
