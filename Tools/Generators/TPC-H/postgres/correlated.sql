-- correlated attributes

-- Q1
explain analyze select count(*) from lineitem, orders, customer where l_quantity > 15 and l_extendedprice < 15000 and l_orderkey = o_orderkey and o_custkey = c_custkey;

-- Q2
explain analyze select count(*) from lineitem, orders, customer where l_quantity < 6 and l_extendedprice > 9500 and l_orderkey = o_orderkey and o_custkey = c_custkey;

-- Q3
explain analyze select count(*) from lineitem, orders, customer where l_quantity < 6 and l_extendedprice > 9525 and l_orderkey = o_orderkey and o_custkey = c_custkey;


-- select the same tuples but using a precomputed attribute  

-- Q1a
explain analyze select count(*) from lineitem2, orders, customer where l_evaluatedpreds = 1 and l_orderkey = o_orderkey and o_custkey = c_custkey;

-- Q2a
explain analyze select count(*) from lineitem3, orders, customer where l_evaluatedpreds = 1 and l_orderkey = o_orderkey and o_custkey = c_custkey;

-- Q3a
explain analyze select count(*) from lineitem4, orders, customer where l_evaluatedpreds = 1 and l_orderkey = o_orderkey and o_custkey = c_custkey;


-- queries on synthesized data
set cpu_tuple_cost = 0.001
set random_page_cost = 1.2
-- otherwise the optimizer will choose a sortmergejoin plan for Q2 which is much
-- slower.


-- Q1
explain analyze select count(*) from r_million as R, s_million as S where R.a=1 and R.c=122 and R.d = S.d;

-- Q2
explain analyze  count(*) from r_million as R, s_million as S where R.a=1 and R.d = S.d;
