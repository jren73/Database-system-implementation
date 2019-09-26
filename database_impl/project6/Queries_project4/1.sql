SELECT SUM(ps_supplycost), ps_suppkey
FROM partsupp 
GROUP BY ps_suppkey

