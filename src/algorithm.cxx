#include "algorithm.hxx"

using namespace ogdf;
typedef LayoutStatistics LS;

uint32_t random_displacement(GraphAttributes& GA, Array<node>& nodes,
                             ArrayBuffer<int, int>& crossings,
                             uint32_t width, uint32_t height, uint32_t steps) {
    // vars used throughout for temp values to revert positions etc.
    uint32_t x,y,id;
    uint32_t size = nodes.size();

    // current min values to check for improvements later
	uint32_t min_overlaps = Math::sum(LS::numberOfNodeOverlaps(GA));
	uint32_t min_nodecrossings = Math::sum(LS::numberOfNodeCrossings(GA));
	uint32_t min_crossings = Math::sum(LS::numberOfCrossings(GA));

    // limits for new position
    List<EdgeElement*> edges;
    uint32_t lower_max;
    uint32_t upper_min;

    // change a random node randomly, keep if graph is improved
    for (uint32_t i = 0; i < steps; ++i) {
        id = rand() % size;
        x = GA.x(nodes[id]);
        y = GA.y(nodes[id]);

        // test all upper and lower nodes connected by an edge for limits
        lower_max = 0;
        nodes[id]->inEdges(edges);
        for (edge e : edges)
            if (GA.y(e->source()) > lower_max)
                lower_max = GA.y(e->source());
        upper_min = height;
        nodes[id]->outEdges(edges);
        for (edge e : edges)
            if (GA.y(e->target()) < upper_min)
                upper_min = GA.y(e->target());
        GA.x(nodes[id]) = rand() % (width + 1);
        ++lower_max; // inc because only inbetween numbers are wanted
        if (upper_min > lower_max) 
            GA.y(nodes[id]) = lower_max + (rand() % (upper_min - lower_max));

        // order of checks is important, numberOfCrossings can segfault
        uint32_t overlaps = Math::sum(LS::numberOfNodeOverlaps(GA));
        if (overlaps <= min_overlaps) {
            uint32_t nodecrossings = Math::sum(LS::numberOfNodeCrossings(GA));
            if (nodecrossings <= min_nodecrossings) {
                double min_angle = Math::minValue(LS::angles(GA));
                if (min_angle > 0.09) {
                    uint32_t crossings = Math::sum(LS::numberOfCrossings(GA));
                    if (crossings <= min_crossings) {
                        min_overlaps = overlaps;
                        min_nodecrossings = nodecrossings;
                        min_crossings = crossings;
                        continue;
                    }
                }
            }
        }

        // revert changes
        GA.x(nodes[id]) = x;
        GA.y(nodes[id]) = y;
    }

    uint32_t sum = Math::sum(crossings);
    return (sum > 0)?(sum / 2):sum;
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

void reduce_crossings(GraphAttributes& GA, Array<node>& nodes, 
                          //Array<edge>& edges, 
                          uint32_t width, 
                          uint32_t height) {
    // initial drawing
    base_layout(GA, nodes, width, height);

    // move nodes to random location within certain boundaries
    //return random_displacement(GA, nodes, crossings, width, height, 0);
}
