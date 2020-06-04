#pragma once

#include <ogdf/basic/graph_generators.h>
#include <ogdf/basic/Layout.h>
#include <ogdf/basic/LayoutStatistics.h>

struct Point{ 
    int x; 
    int y; 

    bool operator==(Point a) {
        if (a.x == x && a.y == y)
            return true;
        return false;
    }

    bool operator!=(Point a) {
        if (a.x == x && a.y == y)
            return false;
        return true;
    }
}; 

uint32_t node_overlaps(ogdf::GraphAttributes& GA, 
                       ogdf::Array<ogdf::node>& nodes, 
                       ogdf::node& n);

uint32_t all_node_overlaps(ogdf::GraphAttributes& GA, 
                       ogdf::Array<ogdf::node>& nodes);

uint32_t node_crossings(ogdf::GraphAttributes& GA, 
                        ogdf::Array<ogdf::edge>& edges, 
                        ogdf::node& n);

uint32_t all_node_crossings(ogdf::GraphAttributes& GA, 
                        ogdf::Array<ogdf::node>& nodes,
                        ogdf::Array<ogdf::edge>& edges);

bool onSegment(Point p, Point q, Point r);

int orientation(Point p, Point q, Point r); 

bool doIntersect(Point p1, Point q1, Point p2, Point q2);

uint32_t intersection_test(ogdf::GraphAttributes& GA, 
                           Point s1,
                           Point t1,
                           ogdf::edge& e2);

uint32_t crossings(ogdf::GraphAttributes& GA, 
                   ogdf::Array<ogdf::edge>& edges, 
                   ogdf::node& n);

uint32_t all_crossings(ogdf::GraphAttributes& GA, 
                   ogdf::Array<ogdf::node>& nodes,
                   ogdf::Array<ogdf::edge>& edges);

bool upward_facing(ogdf::GraphAttributes& GA, 
                   ogdf::Array<ogdf::edge>& edges);

bool within_box(ogdf::GraphAttributes& GA, 
                ogdf::Array<ogdf::node>& nodes,
                uint32_t height,
                uint32_t width);
