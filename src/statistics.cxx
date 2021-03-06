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

    return (sum > 0)?sum / 2:0;
}

uint32_t node_crossings(GraphAttributes& GA, 
                        Array<edge>& edges, 
                        node& n) {
    int32_t x,y;

    for (edge e : edges) {
        if (GA.x(n) <= std::max(GA.x(e->source()), GA.x(e->target())) &&
            GA.x(n) >= std::min(GA.x(e->source()), GA.x(e->target())) &&
            GA.y(n) <= std::max(GA.y(e->source()), GA.y(e->target())) &&
            GA.y(n) >= std::min(GA.y(e->source()), GA.y(e->target()))) { 
            // get vector of edge
            y = GA.y(e->target()) - GA.y(e->source());
            x = GA.x(e->target()) - GA.x(e->source());
            
            // does the node cross with the line of the edge?
            if ((((GA.x(n) - GA.x(e->source())) / x) == 
                ((GA.y(n) - GA.y(e->source())) / y)) ||
                (((GA.x(n) - GA.x(e->source())) / x) &&
                (y == 0) && (GA.y(n) == GA.y(e->source()))) || 
                (((GA.y(n) - GA.y(e->source())) / y) &&
                (x == 0) && (GA.x(n) == GA.x(e->source())))) {
                // is the point within segment?
                if ((GA.y(e->source()) < GA.y(n)) 
                    && (GA.y(e->target()) > GA.y(n))) 
                    return 1;}
        }
    }

    return 0;
}

uint32_t all_node_crossings(GraphAttributes& GA, 
                            Array<node>& nodes,
                            Array<edge>& edges) {
    uint32_t sum = 0;

    #pragma omp parallel for 
    for (node m : nodes) {
        if(node_crossings(GA, edges, m))
            #pragma omp atomic
            ++sum;
    }

    return sum;
}

uint32_t edge_node_crossings(GraphAttributes& GA, 
                             List<edge>& edges, 
                             node& n) {
    int32_t x,y;

    for (edge e : edges) {
        if (GA.x(n) <= std::max(GA.x(e->source()), GA.x(e->target())) &&
            GA.x(n) >= std::min(GA.x(e->source()), GA.x(e->target())) &&
            GA.y(n) <= std::max(GA.y(e->source()), GA.y(e->target())) &&
            GA.y(n) >= std::min(GA.y(e->source()), GA.y(e->target()))) { 
            // get vector of edge
            y = GA.y(e->target()) - GA.y(e->source());
            x = GA.x(e->target()) - GA.x(e->source());
            
            // does the node cross with the line of the edge?
            if ((((GA.x(n) - GA.x(e->source())) / x) == 
                ((GA.y(n) - GA.y(e->source())) / y)) ||
                (((GA.x(n) - GA.x(e->source())) / x) &&
                (y == 0) && (GA.y(n) == GA.y(e->source()))) || 
                (((GA.y(n) - GA.y(e->source())) / y) &&
                (x == 0) && (GA.x(n) == GA.x(e->source())))) {
                // is the point within segment?
                if ((GA.y(e->source()) < GA.y(n)) 
                    && (GA.y(e->target()) > GA.y(n))) 
                    return 1;}
        }
    }

    return 0;
}

uint32_t all_edge_node_crossings(ogdf::GraphAttributes& GA, 
                                 ogdf::Array<ogdf::node>& nodes,
                                 ogdf::Array<ogdf::edge>& edges,
                                 ogdf::node& n) {
    uint32_t sum = 0;

    List<EdgeElement*> con_edges;
    n->adjEdges(con_edges);
    
    sum += node_crossings(GA, edges, n);

    #pragma omp parallel for 
    for (node m : nodes) {
        if(edge_node_crossings(GA, con_edges, m))
            #pragma omp atomic
            ++sum;
    }

    return sum;
}

// mostly taken from 
// https://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/

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
int32_t orientation(Point p, Point q, Point r) 
{ 
    // See https://www.geeksforgeeks.org/orientation-3-ordered-points/ 
    // for details of below formula. 
    int64_t val = ((int64_t) (q.y - p.y) * (r.x - q.x)) - 
                  ((int64_t) (q.x - p.x) * (r.y - q.y)); 
  
    if (val == 0) return 0;  // colinear 
  
    return (val > 0)? 1: 2; // clock or counterclock wise 
} 
  
// The main function that returns true if line segment 'p1q1' 
// and 'p2q2' intersect. 
bool doIntersect(Point p1, Point q1, Point p2, Point q2) 
{ 
    // Find the four orientations needed for general and 
    // special cases 
    int32_t o1 = orientation(p1, q1, p2); 
    int32_t o2 = orientation(p1, q1, q2); 
    int32_t o3 = orientation(p2, q2, p1); 
    int32_t o4 = orientation(p2, q2, q1); 
  
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

int32_t intersection_test(GraphAttributes& GA, 
					   Point s1,
					   Point t1,
                       edge& e2) {
	Point s2, t2;
	
	s2.x = GA.x(e2->source());
	s2.y = GA.y(e2->source());
	t2.x = GA.x(e2->target());
	t2.y = GA.y(e2->target());

    // bounding box intersection?
    if (std::min(s1.x, t1.x) < std::max(t2.x, s2.x) &&
        std::max(t1.x, s1.x) > std::min(s2.x, t2.x) &&
        std::min(s1.y, t1.y) < std::max(t2.y, s2.y) &&
        std::max(t1.y, s1.y) > std::min(s2.y, t2.y)) { 
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

    List<EdgeElement*> adj_edges;
    n1->adjEdges(adj_edges);

    Array<EdgeElement*> adj_edges_arr(adj_edges.size());
    int32_t pos = 0;
    for (EdgeElement* e1 : adj_edges) {
        adj_edges_arr[pos] = e1;
        ++pos;
    }

    #pragma omp parallel for private(s1,t1) reduction(+: sum)
    for (EdgeElement* e1 : adj_edges_arr) {
        s1.x = GA.x(e1->source());
        s1.y = GA.y(e1->source());
        t1.x = GA.x(e1->target());
        t1.y = GA.y(e1->target());
        for (edge e2 : edges)
            sum += intersection_test(GA, s1, t1, e2);
    }

	return sum;
}

uint32_t all_crossings(ogdf::GraphAttributes& GA, 
                   ogdf::Array<ogdf::node>& nodes,
                   ogdf::Array<ogdf::edge>& edges) {
    uint32_t sum = 0;
	for (node n : nodes) {
		sum += crossings(GA, edges, n);
	}
    
    // /4 since each edge is tested twice and each crossing contains two edges 
    return (sum > 0)?sum / 4:0;
}

bool upward_facing(ogdf::GraphAttributes& GA, 
                   ogdf::Array<ogdf::edge>& edges) {
    int32_t downward = 0;
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

