#include <string.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <json/json.h>
#include <json/value.h>
#include <experimental/filesystem>

#include <ogdf/basic/graph_generators.h>
#include <ogdf/basic/Layout.h>
#include <ogdf/basic/LayoutStatistics.h>

namespace fs = std::experimental::filesystem;
using namespace ogdf;

void handle_graph(std::string path) {
	// reading josn
	std::ifstream in(path, std::ifstream::binary);
	Json::Value json;
	in >> json;	

    // building graph
	Graph G;

	GraphAttributes GA(G,
	GraphAttributes::nodeGraphics |
	GraphAttributes::edgeGraphics |
	GraphAttributes::nodeId |
	GraphAttributes::nodeLabel |
	GraphAttributes::edgeStyle |
	GraphAttributes::nodeStyle |
	GraphAttributes::nodeTemplate);

    uint32_t width = json["width"].asUInt();
    uint32_t height = json["height"].asUInt();

    // fill graph with json data
    std::vector<node> nodes(0);
    for (Json::Value j : json["nodes"]) {
         node n = G.newNode(j["id"].asUInt());
         GA.idNode(n) = j["id"].asUInt();
         GA.x(n) = j["x"].asUInt();
         GA.y(n) = j["y"].asUInt();
         nodes.push_back(n);
    } 

    for (Json::Value j : json["edges"]) {
         edge e = G.newEdge(nodes[j["source"].asUInt()],
                            nodes[j["target"].asUInt()],
                            j["id"].asUInt());
    }

	// call upon graph algorithm

	// terminal output
    GA.setAllWidth(0.5);
    GA.setAllHeight(0.5);

	auto crossings = LayoutStatistics::numberOfCrossings(GA);
    int cross = crossings[0];
    if (cross > 0) 
      cross /= 2;
	auto overlaps = LayoutStatistics::numberOfNodeOverlaps(GA);
    int over = overlaps[0];
    if (over > 0)
        over /= 2;
	auto nodecrossings = LayoutStatistics::numberOfNodeCrossings(GA);
    int ncross = nodecrossings[0];

    int downward = 0;
    Array<edge, int> edges(G.numberOfEdges());
    G.allEdges(edges);
    for (edge e : edges) {
        if (GA.y(e->source()) >= GA.y(e->target()))
            ++downward;
    }

    std::cout << "-------------------------------------" << std::endl;
	std::cout << "Graph: " << path << std::endl;
	std::cout << cross << " crossing(s)!" << std::endl;
	std::cout << ncross << " node crossing(s)!" << std::endl;
	std::cout << over << " overlap(s)!" << std::endl;
	std::cout << downward << " not upward facing edge(s)!" << std::endl;

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

    // exiting
    nodes.clear();
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
