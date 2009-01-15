-- Jan. 2009, M. Spiekermann
--
-- Sanity checks for a tpc database


-- Compute cards

select count(*) as "Region_Card = 5 "  from region;
select count(*) as "Nation_Card  = 25" from nation;

-- Compute scale factors

select count(*) / 10000.0 as  SF from supplier;
select count(*) / 150000.0 as SF from customer;
select count(*) / 200000.0 as SF from part;
select count(*) / 800000.0 as SF from partsupp;
select count(*) / 1500000.0 as SF from orders;
select count(*) / 6000000.0 as "SF (small deviations are allowed here)" from lineitem;




