((a.node_type() == IRNodeType::Add) && (
 rewrite((((min((x + c0), y) + z) + c1)/c2), ((min((y + c1), x) + z)/c2), ((c0 + c1) == 0)) ||
 rewrite((((x - ((y*c0) + z)) + c1)/c0), (((x + (c1 - z))/c0) - y)) ||
 rewrite(((((x + (y*c0)) - z) + c1)/c0), (y + (((x - z) + c1)/c0))) ||
 rewrite((((((x*c0) + y) - z) + c1)/c0), ((((c1 + y) - z)/c0) + x)) ||
 rewrite(((min((x*c0), c1) + c1)/c0), min(x, c1), ((((((0 <= c1) && (c1 <= 2)) && (c1 <= ((c1 + 1)*c0))) && (((c1*2) + 1) <= ((c1 + 1)*c0))) && ((max(c1, 1) + 1) <= c0)) && ((c0*c1) <= (c1*2)))))) ||
((a.node_type() == IRNodeType::Min) && (
 rewrite((min(x, ((y*c0) + c1))/c0), min(y, (x/c0)), (((0 <= c1) && (c1 <= 10)) && ((max(c1, 0) + 1) <= c0))) ||
 rewrite((min(((y*c0) + c1), x)/c0), min((x/c0), y), (((0 <= c1) && (c1 <= 10)) && ((max(c1, 0) + 1) <= c0))) ||
 rewrite((min((x*c0), c1)/c0), min(x, -1), (((0 <= (c0 + c1)) && (1 <= c0)) && (c1 <= -1))) ||
 rewrite((min((x*c0), ((y*c0) + c1))/c0), min(x, (y + 0)), (((0 <= c1) && ((c1 + 1) <= (c0*2))) && ((max(c1, 0) + 1) <= c0))))) ||
((a.node_type() == IRNodeType::Max) && (
 rewrite((max((x*c0), c1)/c0), max(x, 0), ((0 <= c1) && ((max(c1, 0) + 1) <= c0))) ||
 rewrite((max((x*c0), c1)/c0), max(x, c1), ((((-1 <= c1) && (0 <= (c0 + c1))) && (1 <= c0)) && (c1 <= 0)))))