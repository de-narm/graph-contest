#pragma once

#include <iostream>
#include <vector>
#include <set>

#include <ogdf/basic/graph_generators.h>
#include <ogdf/basic/Layout.h>
#include <ogdf/basic/LayoutStatistics.h>
#include <ogdf/layered/MedianHeuristic.h>
#include <ogdf/layered/OptimalHierarchyLayout.h>
#include <ogdf/layered/OptimalRanking.h>
#include <ogdf/layered/SugiyamaLayout.h>

#include <ogdf/energybased/spring_embedder/SpringEmbedderBase.h>
#include <ogdf/energybased/SpringEmbedderGridVariant.h>

#include "statistics.hxx"

void random_displacement(ogdf::GraphAttributes& GA, 
                         ogdf::Array<ogdf::node>& nodes,
                         ogdf::Array<ogdf::edge>& edges,
                         uint32_t width, uint32_t height,
                         uint32_t steps);

void base_layout(ogdf::GraphAttributes& GA, 
                 ogdf::Array<ogdf::node>& nodes,
                 uint32_t width, uint32_t height);

void sugiyama_layout(ogdf::GraphAttributes& GA, 
                     ogdf::Array<ogdf::node>& nodes,
                     uint32_t width, uint32_t height);

void reduce_crossings(ogdf::GraphAttributes& GA, 
                      ogdf::Array<ogdf::node>& nodes,
                      ogdf::Array<ogdf::edge>& edges,
                      uint32_t width, uint32_t height);
