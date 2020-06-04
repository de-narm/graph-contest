#pragma once

#include <iostream>
#include <vector>
#include <set>

#include <ogdf/basic/graph_generators.h>
#include <ogdf/basic/Layout.h>
#include <ogdf/basic/LayoutStatistics.h>

#include "statistics.hxx"

uint32_t random_displacement(ogdf::GraphAttributes& GA, 
                             ogdf::ArrayBuffer<int, int>& crossings,
                             ogdf::Array<ogdf::node>& nodes,
                             uint32_t width, uint32_t height);

void base_layout(ogdf::GraphAttributes& GA, 
                 ogdf::Array<ogdf::node>& nodes,
                 uint32_t width, uint32_t height);

void reduce_crossings(ogdf::GraphAttributes& GA, 
                      ogdf::Array<ogdf::node>& nodes,
                      //ogdf::Array<ogdf::edge>& edges,
                      uint32_t width, uint32_t height);
