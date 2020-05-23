#include "algorithm.hxx"

using namespace ogdf;
typedef LayoutStatistics LS;

void random_displacement(GraphAttributes& GA, Array<node>& nodes, 
                         uint32_t width, uint32_t height, uint32_t steps) {
    // vars used throughout for temp values to revert positions etc.
    uint32_t x,y,id;
    uint32_t size = nodes.size();

    // current min values to check for improvements later
	uint32_t min_overlaps = Math::sum(LS::numberOfNodeOverlaps(GA));
	uint32_t min_nodecrossings = Math::sum(LS::numberOfNodeCrossings(GA));
	uint32_t min_crossings = Math::sum(LS::numberOfCrossings(GA));

    // change a random node randomly, keep if graph is improved
    for (uint32_t i = 0; i < steps; ++i) {
        id = rand() % size;
        x = GA.x(nodes[id]);
        y = GA.y(nodes[id]);
        GA.x(nodes[id]) = rand() % (height + 1);
        GA.y(nodes[id]) = rand() % (width + 1);

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
    float spacing = 0;
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


    // stretch layers to maximum y-distance between them
    spacing = height / i;
    for (node n : nodes) {
        GA.y(n) *= spacing;
    }
}

void reduce_crossings(GraphAttributes& GA, Array<node>& nodes, 
                      uint32_t width, uint32_t height) {
    base_layout(GA, nodes, width, height);
    random_displacement(GA, nodes, width, height, 0);
}
