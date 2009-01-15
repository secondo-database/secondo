-- Jan. 2009, M. Spiekermann
--

create table lineitem2 as select * from lineitem;
insert into lineitem2 select * from lineitem;

