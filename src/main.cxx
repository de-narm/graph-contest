#include <string.h>
#include <string>
#include <iostream>
#include <fstream>
#include <json/json.h>
#include <json/value.h>
#include <experimental/filesystem>

#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/basic/Layout.h>
#include <ogdf/basic/LayoutStatistics.h>
#include <ogdf/fileformats/GraphIO.h>

#include "algorithm.hxx"

namespace fs = std::experimental::filesystem;
using namespace ogdf;

void handle_graph(std::string path) {
	// reading json
	std::ifstream in(path, std::ifstream::binary);
	Json::Value json;
	in >> json;	

    // building graph
	Graph G;

	GraphAttributes GA(G,
	GraphAttributes::nodeGraphics |
	GraphAttributes::edgeGraphics |
	GraphAttributes::nodeStyle |
	GraphAttributes::edgeStyle |
	GraphAttributes::nodeId);

    uint32_t width = json["width"].asUInt();
    uint32_t height = json["height"].asUInt();

    // fill graph with json data
    for (Json::Value j : json["nodes"]) {
         node n = G.newNode();
         GA.idNode(n) = j["id"].asUInt();
         // current base layout ignores initial values
         //GA.x(n) = j["x"].asUInt();
         //GA.y(n) = j["y"].asUInt();
         GA.width(n) = 0.1;
         GA.height(n) = 0.1;
         GA.strokeWidth(n) = 0.1;
    } 

    Array<node, int> nodes(G.numberOfNodes());
    G.allNodes(nodes);
    for (Json::Value j : json["edges"]) {
         edge e = G.newEdge(nodes[j["source"].asUInt()],
                            nodes[j["target"].asUInt()]);
         GA.strokeWidth(e) = 0.02;
    }

	// call upon graph algorithm
    reduce_crossings(GA, nodes, width, height);

	// terminal output
	auto crossings = LayoutStatistics::numberOfCrossings(GA);
    int cross = Math::sum(crossings);
    if (cross > 0) 
         cross /= 2;

	auto overlaps = LayoutStatistics::numberOfNodeOverlaps(GA);
    int over = Math::sum(overlaps);
    if (over > 0)
         over /= 2;

	auto nodecrossings = LayoutStatistics::numberOfNodeCrossings(GA);
    int ncross = Math::sum(nodecrossings);

    int downward = 0;
    Array<edge, int> edges(G.numberOfEdges());
    G.allEdges(edges);
    for (edge e : edges) {
        if (GA.y(e->source()) >= GA.y(e->target()))
            ++downward;
    }

    uint32_t min_x = height;
    uint32_t max_x = 0;
    uint32_t min_y = width;
    uint32_t max_y = 0;
    uint32_t x;
    uint32_t y;
    for (node n : nodes) {
        x = GA.x(n);
        y = GA.y(n);
        if (x > max_x)
            max_x = x;
        if (x < min_x)
            min_x = x;
        if (y > max_y)
            max_y = y;
        if (y < min_y)
            min_y = y;
    }

    double min_angle = Math::minValue(LayoutStatistics::angles(GA));

    std::cout << "-------------------------------------" << std::endl;
	std::cout << "Graph: " << path << std::endl;
	std::cout << cross << " crossing(s)!" << std::endl;
    if (ncross != 0 || over != 0 || downward != 0 ||
        max_x > height || max_y > width) {
	    std::cout << min_angle << " min radian angle!" << std::endl;
    	std::cout << ncross << " node crossing(s)!" << std::endl;
        std::cout << over << " overlap(s)!" << std::endl;
        std::cout << downward << " not upward facing edge(s)!" << std::endl;
        std::cout << "X: [" << min_x << "," << max_x << "] (" << height << ")" 
                  << std::endl;
        std::cout << "Y: [" << min_y << "," << max_y << "] (" << width << ")" 
                  << std::endl;
    }

    // save graph in json format 
    uint32_t id;
    for (node n : nodes) {
        id = GA.idNode(n);
        // just in case they aren't sorted
        json["nodes"][id]["id"] = id;
        json["nodes"][id]["x"] = (uint32_t) GA.x(n);
        json["nodes"][id]["y"] = (uint32_t) GA.y(n);
    } 
    
    path.replace(path.end()-4, path.end(), "out.json");
	std::ofstream out(path, std::ifstream::binary);
    out << json;

    // svg image
    GraphIO::write(GA, path.append(".svg"), GraphIO::drawSVG);

    // exiting
}

void traverse_dir(std::string path) {
    // sort diretory entries
    std::vector<fs::directory_entry> paths(0);
    for (const auto & entry : fs::directory_iterator(path))
        paths.push_back(entry);
    std::sort(paths.begin(), paths.end());

    // call the file function for each file in the dir
    std::size_t found;
    std::string filepath;
    for (const auto & entry : paths) {
        filepath = entry.path();
        // filter previous results
        found = filepath.find(".out.");
        if (found == std::string::npos)
            handle_graph(filepath);
    }
}

int main(int argc, char** argv) {
    // handle input flags (-d "dir" -f "file")
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-f")) {
            std::string path(argv[i+1]);
            handle_graph(path); 
        } else if (!strcmp(argv[i], "-d")) {
            std::string path(argv[i+1]);
            traverse_dir(path);
        }
    }
    return 0;
}
