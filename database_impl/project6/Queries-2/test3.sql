SELECT l_orderkey
FROM lineitem, orders
WHERE l_orderkey=o_orderkey AND o_orderkey < 100;
