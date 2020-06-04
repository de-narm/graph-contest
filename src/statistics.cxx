// most if not all of the following only works with upwards planar drawings

#include "statistics.hxx"

using namespace ogdf;


uint32_t node_overlaps(GraphAttributes& GA, Array<node>& nodes, 
                       node& n) {
    uint32_t overlaps = 0;
    for (node m : nodes) {
        if (GA.x(m) == GA.x(n) && GA.y(m) == GA.y(n))
            ++overlaps;
    }
    
    // once the node is compared to itself
    return (overlaps - 1);
}

uint32_t all_node_overlaps(GraphAttributes& GA, 
                       Array<node>& nodes) {
    uint32_t sum = 0;
    for (node m : nodes) {
        sum += node_overlaps(GA, nodes, m);
    }

    return sum / 2;
}

uint32_t node_crossings(GraphAttributes& GA, 
                        Array<edge>& edges, 
                        node& n) {
    uint32_t crossings = 0;
    uint32_t x,y;
    for (edge e : edges) {
        // point == source/target?
        if (((GA.x(n) == GA.x(e->source())) && 
            (GA.y(n) == GA.y(e->source()))) ||
            ((GA.x(n) == GA.x(e->target())) && 
            (GA.y(n) == GA.y(e->target()))))
            continue;

        // get vector of edge
        y = GA.y(e->target()) - GA.y(e->source());
        x = GA.x(e->target()) - GA.x(e->source());
        
        // does the node cross with the line of the edge?
        if ((((GA.x(n) - GA.x(e->source())) / x) == 
            ((GA.y(n) - GA.y(e->source())) / y)) ||
            (((GA.x(n) - GA.x(e->source())) / x) &&
            (y == 0) && (GA.y(n) == GA.y(e->source()))) || 
            (((GA.y(n) - GA.y(e->source())) / y) &&
            (x == 0) && (GA.x(n) == GA.x(e->source()))))
            // is the point within segment?
            if ((GA.y(e->source()) >= GA.y(n) 
               && GA.y(e->target()) <= GA.y(n)) 
               || (GA.y(e->source()) <= GA.y(n) 
               && GA.y(e->target()) >= GA.y(n))) 
                ++crossings;
    }

    return crossings;
}

uint32_t all_node_crossings(GraphAttributes& GA, 
                        Array<node>& nodes,
                        Array<edge>& edges) {
    uint32_t sum = 0;
    for (node m : nodes) {
        sum += node_crossings(GA, edges, m);
    }

    return sum;
}

// taken from https://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/
// Given three colinear points p, q, r, the function checks if 
// point q lies on line segment 'pr' 
bool onSegment(Point p, Point q, Point r) 
{ 
    if (q.x <= max(p.x, r.x) && q.x >= min(p.x, r.x) && 
        q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y)) 
       return true; 
  
    return false; 
} 
  
// To find orientation of ordered triplet (p, q, r). 
// The function returns following values 
// 0 --> p, q and r are colinear 
// 1 --> Clockwise 
// 2 --> Counterclockwise 
int orientation(Point p, Point q, Point r) 
{ 
    // See https://www.geeksforgeeks.org/orientation-3-ordered-points/ 
    // for details of below formula. 
    int val = (q.y - p.y) * (r.x - q.x) - 
              (q.x - p.x) * (r.y - q.y); 
  
    if (val == 0) return 0;  // colinear 
  
    return (val > 0)? 1: 2; // clock or counterclock wise 
} 
  
// The main function that returns true if line segment 'p1q1' 
// and 'p2q2' intersect. 
bool doIntersect(Point p1, Point q1, Point p2, Point q2) 
{ 
    // Find the four orientations needed for general and 
    // special cases 
    int o1 = orientation(p1, q1, p2); 
    int o2 = orientation(p1, q1, q2); 
    int o3 = orientation(p2, q2, p1); 
    int o4 = orientation(p2, q2, q1); 
  
    // General case 
    if (o1 != o2 && o3 != o4) 
        return true; 
  
    // Special Cases 
    // p1, q1 and p2 are colinear and p2 lies on segment p1q1 
    if (o1 == 0 && onSegment(p1, p2, q1)) return true; 
  
    // p1, q1 and q2 are colinear and q2 lies on segment p1q1 
    if (o2 == 0 && onSegment(p1, q2, q1)) return true; 
  
    // p2, q2 and p1 are colinear and p1 lies on segment p2q2 
    if (o3 == 0 && onSegment(p2, p1, q2)) return true; 
  
     // p2, q2 and q1 are colinear and q1 lies on segment p2q2 
    if (o4 == 0 && onSegment(p2, q1, q2)) return true; 
  
    return false; // Doesn't fall in any of the above cases 
}
// end of copied code 

uint32_t intersection_test(GraphAttributes& GA, 
						   Point s1,
						   Point t1,
                           edge& e2) {
	Point s2, t2;
	
	s2.x = GA.x(e2->source());
	s2.y = GA.y(e2->source());
	t2.x = GA.x(e2->target());
	t2.y = GA.y(e2->target());

    // bounding box intersection?
    if (s1.x <= t2.x &&
        t1.x >= s2.x &&
        s1.y <= t2.y &&
        t1.y >= s2.y) { 
        // not with same nodes connected?
        if (s1 == s2 || s1 == t2 || t1 == s2 || t1 == t2)
            return 0;
        // intersection?
        if (doIntersect(s1, t1, s2, t2)) 
            return 1;
    }

    return 0;
}

uint32_t crossings(ogdf::GraphAttributes& GA, 
                   ogdf::Array<ogdf::edge>& edges, 
                   ogdf::node& n1) {
	uint32_t sum = 0;
	Point s1, t1;

	s1.x = GA.x(n1);
	s1.y = GA.y(n1);

    List<EdgeElement*>* edge_in = new List<EdgeElement*>();
    n1->adjEdges(*edge_in);

    List<EdgeElement*>* edge_out = new List<EdgeElement*>();
    n1->adjEdges(*edge_out);

	for (EdgeElement* e2 : *edge_in) {
		t1.x = GA.x(e2->source());
		t1.y = GA.y(e2->source());
		for (edge e2 : edges) {
			sum += intersection_test(GA, s1, t1, e2);
		}	
	}

	for (EdgeElement* e2 : *edge_out) {
		t1.x = GA.x(e2->target());
		t1.y = GA.y(e2->target());
		for (edge e2 : edges) {
			sum += intersection_test(GA, s1, t1, e2);
		}	
	}

    // once the edge crosses itself
	return sum;
}

uint32_t all_crossings(ogdf::GraphAttributes& GA, 
                   ogdf::Array<ogdf::node>& nodes,
                   ogdf::Array<ogdf::edge>& edges) {
    uint32_t sum = 0;
	for (node n : nodes) {
		sum += crossings(GA, edges, n);
	}

    return sum / 2;
}

bool upward_facing(ogdf::GraphAttributes& GA, 
                   ogdf::Array<ogdf::edge>& edges) {
    int downward = 0;
    for (edge e : edges) {
        if (GA.y(e->source()) >= GA.y(e->target()))
            ++downward;
    }

    return (downward > 0)?false:true;
}

bool within_box(ogdf::GraphAttributes& GA, 
                ogdf::Array<ogdf::node>& nodes,
                uint32_t height,
                uint32_t width) {
    uint32_t max_x = 0;
    uint32_t max_y = 0;
    uint32_t x;
    uint32_t y;
    for (node n : nodes) {
        x = GA.x(n);
        y = GA.y(n);
        if (x > max_x)
            max_x = x;
        if (y > max_y)
            max_y = y;
    }
    
    // min values always >= 0 since I mostly use uint32_t
    return ((max_x <= width) && (max_y <= height));
}

