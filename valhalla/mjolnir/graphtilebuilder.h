#ifndef VALHALLA_MJOLNIR_GRAPHTILEBUILDER_H_
#define VALHALLA_MJOLNIR_GRAPHTILEBUILDER_H_

#include <boost/functional/hash.hpp>
#include <fstream>
#include <iostream>
#include <list>
#include <utility>
#include <algorithm>
#include <string>
#include <memory>
#include <list>
#include <unordered_set>
#include <unordered_map>

#include <valhalla/baldr/graphid.h>
#include <valhalla/baldr/graphtile.h>
#include <valhalla/mjolnir/graphtileheaderbuilder.h>
#include <valhalla/mjolnir/nodeinfobuilder.h>
#include <valhalla/mjolnir/directededgebuilder.h>
#include <valhalla/mjolnir/edgeinfobuilder.h>
#include <valhalla/baldr/tilehierarchy.h>

namespace valhalla {
namespace mjolnir {

using edge_tuple = std::tuple<uint32_t, baldr::GraphId, baldr::GraphId>;

/**
 * Graph information for a tile within the Tiled Hierarchical Graph.
 * @author  David W. Nesbitt
 */
class GraphTileBuilder : public baldr::GraphTile {
 public:
  /**
   * Constructor
   */
  GraphTileBuilder();

  /**
   * Constructor given an existing tile. This is used to read in the tile
   * data and then add to it (e.g. adding node connections between hierarchy
   * levels.
   * @param  basedir  Base directory path
   * @param  graphid  GraphId used to determine the tileid and level
   */
  GraphTileBuilder(const baldr::TileHierarchy& hierarchy, const GraphId& graphid);

  /**
   * Output the tile to file. Stores as binary data.
   * @param  graphid  GraphID to store.
   * @param  hierarchy  Gives info about number of tiles per level
   */
  void StoreTileData(const baldr::TileHierarchy& hierarchy,
                     const baldr::GraphId& graphid);

  /**
   * Update a graph tile with new header, nodes, and directed edges. This
   * is used to add directed edges connecting two hierarchy levels.
   * @param  hierarchy      How the tiles are setup on disk
   * @param  hdr            Update header
   * @param  nodes          Update list of nodes
   * @param  directededges  Updated list of edges.
   */
  void Update(const baldr::TileHierarchy& hierarchy,
              const GraphTileHeaderBuilder& hdr,
              const std::vector<NodeInfoBuilder>& nodes,
              const std::vector<DirectedEdgeBuilder> directededges);

  /**
   * Add a node and its outbound edges.
   */
  void AddNodeAndDirectedEdges(
      const NodeInfoBuilder& node,
      const std::vector<DirectedEdgeBuilder>& directededges);

  /**
   * Add edge info to the tile.
   */
  uint32_t AddEdgeInfo(const uint32_t edgeindex, const baldr::GraphId& nodea,
                       const baldr::GraphId& nodeb,
                       const std::vector<PointLL>& lls,
                       const std::vector<std::string>& names);

  /**
   * Gets a builder for a node from an existing tile.
   * @param  idx  Index of the node within the tile.
   */
  NodeInfoBuilder& node(const size_t idx);

  /**
   * Gets a builder for a directed edge from existing tile data.
   * @param  idx  Index of the directed edge within the tile.
   */
  DirectedEdgeBuilder& directededge(const size_t idx);

 protected:

  struct EdgeTupleHasher {
    std::size_t operator()(const edge_tuple& k) const {
      std::size_t seed = 13;
      boost::hash_combine(seed, index_hasher(std::get<0>(k)));
      boost::hash_combine(seed, id_hasher(std::get<1>(k)));
      boost::hash_combine(seed, id_hasher(std::get<2>(k)));
      return seed;
    }
    //function to hash each id
    std::hash<uint32_t> index_hasher;
    std::hash<valhalla::baldr::GraphId> id_hasher;
  };

  // Edge tuple for sharing edges that have common nodes and edgeindex
  static edge_tuple EdgeTuple(const uint32_t edgeindex,
                       const valhalla::baldr::GraphId& nodea,
                       const valhalla::baldr::GraphId& nodeb) {
    return (nodea < nodeb) ? std::make_tuple(edgeindex, nodea, nodeb):
        std::make_tuple(edgeindex, nodeb, nodea);
  }

  // Write all edgeinfo items to specified stream
  void SerializeEdgeInfosToOstream(std::ostream& out);

  // Write all textlist items to specified stream
  void SerializeTextListToOstream(std::ostream& out);

  // Header information for the tile
  GraphTileHeaderBuilder header_builder_;

  // List of nodes. This is a fixed size structure so it can be
  // indexed directly.
  std::vector<NodeInfoBuilder> nodes_builder_;

  // List of directed edges. This is a fixed size structure so it can be
  // indexed directly.
  std::vector<DirectedEdgeBuilder> directededges_builder_;

  // Edge info offset and map
  size_t edge_info_offset_ = 0;
  std::unordered_map<edge_tuple, size_t, EdgeTupleHasher> edge_offset_map;

  // The edgeinfo list
  std::list<EdgeInfoBuilder> edgeinfo_list_;

   // Text list offset and map
   uint32_t text_list_offset_ = 0;
   std::unordered_map<std::string, uint32_t> text_offset_map;

   // Text list. List of names used within this tile
   std::list<std::string> textlistbuilder_;
};

}
}

#endif  // VALHALLA_MJOLNIR_GRAPHTILEBUILDER_H_

