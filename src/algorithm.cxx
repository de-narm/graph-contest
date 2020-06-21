#include "algorithm.hxx"

using namespace ogdf;
typedef LayoutStatistics LS;

void random_displacement(GraphAttributes& GA, Array<node>& nodes,
                         Array<edge>& edges,
                         uint32_t width, uint32_t height, uint32_t steps) {
    // vars used throughout for temp values to revert positions etc.
    uint32_t x,y,id, random;
    // vars to compare positions
    uint32_t old_overlaps, old_nodecrossings, old_crossings, new_crossings;
//    uint32_t size = nodes.size();

    // limits for new position
    List<EdgeElement*> con_edges;
    int32_t lower_max, upper_min;

    // base value for crossings; > 0 == each node pickable for displacement
    uint32_t cross_weight = (steps/10);
    
    // all initial edge crossing values
    std::vector<uint32_t> cross_list;
    uint32_t cross_sum = 1;
    uint32_t cross_tmp = 0;
    for (auto n : nodes) {
        cross_tmp = (crossings(GA, edges, n) + cross_weight);
        cross_list.push_back(cross_tmp); 
        cross_sum += cross_tmp;
    }

    // change a random node randomly, keep if graph is improved
    for (uint32_t i = 0; i < steps; ++i) {
        // get nodes with more crossings more often
        random = rand() % cross_sum;
        id = 0;
        for (uint32_t c : cross_list) {
            if(c >= random) {
                break;
            } else {
               random -= c; 
            }
            ++id;
        }

        // current min values to check for improvements later
        old_overlaps = node_overlaps(GA, nodes, nodes[id]);
        old_nodecrossings = all_edge_node_crossings(GA, nodes, edges,
                                                    nodes[id]);
        old_crossings = (crossings(GA, edges, nodes[id]) + cross_weight);

        // update crossing list; other node movements could have changed it
        cross_tmp = cross_list[id];
        cross_list[id] = old_crossings; 
        cross_sum -= (cross_tmp - old_crossings);

        // not moving 0-nodes has proven better for smaller graphs
        if (old_overlaps == 0 && old_nodecrossings == 0 && 
            old_crossings <= cross_weight) {
            if (cross_weight > 0)
                --cross_weight;
            continue;
        }

        x = GA.x(nodes[id]);
        y = GA.y(nodes[id]);

        // test all upper and lower nodes connected by an edge for limits
        lower_max = -1;
        nodes[id]->inEdges(con_edges);
        for (edge e : con_edges)
            if (GA.y(e->source()) > lower_max)
                lower_max = GA.y(e->source());
        upper_min = (height + 1);
        nodes[id]->outEdges(con_edges);
        for (edge e : con_edges)
            if (GA.y(e->target()) < upper_min)
                upper_min = GA.y(e->target());
        ++lower_max; // inc because only inbetween numbers are wanted

        // set new random values
        GA.x(nodes[id]) = rand() % (width + 1);
        if (upper_min > lower_max) 
            GA.y(nodes[id]) = lower_max + (rand() % (upper_min - lower_max));

        // update crossing list
        new_crossings = (crossings(GA, edges, nodes[id]) + cross_weight); 
        cross_tmp = cross_list[id];
        cross_list[id] = new_crossings; 
        cross_sum -= (cross_tmp - new_crossings);

        // check for improvements
        if (node_overlaps(GA, nodes, nodes[id]) <= old_overlaps) {
            //if (node_crossings(GA, edges, nodes[id]) <= old_nodecrossings) {
            if (all_edge_node_crossings(GA, nodes, edges, nodes[id]) <=
                old_nodecrossings) {
                if (new_crossings <= old_crossings) 
                    continue;
                // regardless of crossings, keep changes to improve correctness
                if (old_nodecrossings > 0)
                    continue;
            }
            if (old_overlaps > 0)
                continue;
        }

        // update crossing list; other node movements could have changed it
        cross_tmp = cross_list[id];
        cross_list[id] = old_crossings; 
        cross_sum -= (cross_tmp - old_crossings);

        // revert changes
        GA.x(nodes[id]) = x;
        GA.y(nodes[id]) = y;
    }
}

void base_layout(GraphAttributes& GA, Array<node>& nodes, 
                 uint32_t width, uint32_t height) {
    // both sets will later be alternated
    std::set<node> setA;
    std::set<node> setB;

    // fill setA with all nodes belonging to the very bottom
    for (node n : nodes)
        if (n->indeg() == 0)
            setA.insert(n);
            
    /* place all nodes in the same layer with almost maximum x-distance between
       them and find all nodes the current set has edges pointed to */
    std::set<node>* current;
    std::set<node>* next;
    List<EdgeElement*> out_edges;
    uint32_t i = 0;
    uint32_t j = 0;
    uint32_t spacing = 0;
    do {
        // alternating
        if ((i % 2) == 0) {
            current = &setA;
            next    = &setB;
        } else {
            current = &setB;
            next    = &setA;
        }

        // place on layer
        spacing = width / current->size();
        j = 0;
        for (node n : *current) {
            GA.x(n) = j * spacing;
            GA.y(n) = i;
            ++j;
        }

        // determine next set
        next->clear();
        for (node n : *current) {
            n->outEdges(out_edges);
            for (edge e : out_edges)
                next->insert(e->target());
        }

        ++i;
    } while (!next->empty());


    // stretch layers to almost maximum y-distance between them
    spacing = height / i;
    for (node n : nodes) 
        GA.y(n) *= spacing;
}

void sugiyama_layout(GraphAttributes& GA, Array<node>& nodes, 
                     uint32_t width, uint32_t height) {
    // call basic Sugiyama algorithm
    SugiyamaLayout SL;
    SL.setRanking(new OptimalRanking);
    SL.setCrossMin(new MedianHeuristic);

    OptimalHierarchyLayout* ohl = new OptimalHierarchyLayout;
    ohl->layerDistance(1.0);
    ohl->nodeDistance(1.0);
    ohl->weightBalancing(0.8);
    SL.setLayout(ohl);

    SL.call(GA);

    // find maximum values
    uint32_t max_x = 1;
    uint32_t max_y = 1;
    uint32_t x, y;

    for (node n : nodes) {
        x = GA.x(n); 
        y = GA.y(n); 
        if (x > max_x) 
            max_x = x;
        if (y > max_y) 
            max_y = y;
    }

    // stretch/compress to box limit
    double x_factor = (double) width / max_x;
    double y_factor = (double) height / max_y;

    for (node n : nodes) {
       GA.x(n) = (int) (((int) GA.x(n)) * x_factor);
       GA.y(n) = (int) (((int) GA.y(n)) * y_factor);
    }
}

void reduce_crossings(GraphAttributes& GA, Array<node>& nodes, 
                      Array<edge>& edges, 
                      uint32_t width, 
                      uint32_t height) {
    // initial drawing
    //base_layout(GA, nodes, width, height);
    sugiyama_layout(GA, nodes, width, height);

    // move nodes to random location within certain boundaries
    random_displacement(GA, nodes, edges, width, height, 100000);
}
