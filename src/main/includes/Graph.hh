// Copyright (C) 2015 Dominik Dahlem <Dominik.Dahlem@gmail.com>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

/** @file Graph.hh
 * Declaration of the graph functionality
 *
 * @author Dominik Dahlem
 */
#ifndef __GRAPH_HH__
#define __GRAPH_HH__

#if HAVE_CONFIG_H
# include <config.h>
#endif

#ifndef __STDC_CONSTANT_MACROS
# define __STDC_CONSTANT_MACROS
#endif /* __STDC_CONSTANT_MACROS */

#ifndef NDEBUG
# include <algorithm>
# include <iostream>
# include <iterator>
#endif /* NDEBUG */

#include <sstream>

#include <boost/cstdint.hpp>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>

#include <boost/algorithm/string/split.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>

#include "Types.hh"
namespace itypes = types;

namespace graph
{

typedef boost::tokenizer <boost::escaped_list_separator <char> > Tokenizer;


/** @struct VertexProperties
 * Structure to define a vertex in the tree
 */
struct VertexProperties
{
  boost::uint32_t id;
  double level;
  std::string name;
};

/** @typedef Graph
 * Specifies the graph to be used as a tree.
 */
typedef boost::adjacency_list<
  boost::vecS,
  boost::vecS,
  boost::bidirectionalS,
  VertexProperties> Graph;

/** @typedef Vertex
 * Specifies the vertex descriptor of a graph
 */
typedef boost::graph_traits <Graph>::vertex_descriptor Vertex;

/** @typedef Edge
 * Specifies the edge descriptor of a graph
 */
typedef boost::graph_traits <Graph>::edge_descriptor Edge;

/** @typedef VertexIterator
 * Specifies the iterator for the vertices
 */
typedef boost::graph_traits <Graph>::vertex_iterator VertexIterator;

/** @typedef EdgeIterator
 * Specifies the iterator for the edges
 */
typedef boost::graph_traits<Graph>::edge_iterator EdgeIterator;

/** @typedef OutEdgeIterator
 * Specifies the iterator for the out degree edges
 */
typedef boost::graph_traits <Graph>::out_edge_iterator OutEdgeIterator;

/** @typedef InEdgeIterator
 * Specifies the iterator for the in degree edges
 */
typedef boost::graph_traits <Graph>::in_edge_iterator InEdgeIterator;

/** @typedef Color
 * Specifies the color traits
 */
typedef boost::color_traits <boost::default_color_type> Color;

/** @typedef NamePropertyMap
 * Defines the property map for names of the vertices
 */
typedef boost::property_map<Graph, std::string VertexProperties::*>::type NamePropertyMap;

/** @typedef IDPropertyMap
 * Defines the property map for IDs of the vertices
 */
typedef boost::property_map<Graph, boost::uint32_t VertexProperties::*>::type IDPropertyMap;


Vertex find(const std::string &p_symbol, Graph &p_graph, Vertex &p_vertex)
{
  BOOST_FOREACH(Edge e, (boost::out_edges(p_vertex, p_graph))) {
    bool found = false;

    if (p_graph[target(e, p_graph)].name == p_symbol) {
      found = true;
    }

    if (found) {
      return target(e, p_graph);
    }
  }

  return NULL;
}

void add(const itypes::StringVector &p_cats, Graph &p_graph, Vertex &p_root)
{
  Vertex curV = p_root;
  Vertex nextV = NULL;
  boost::uint32_t i = 0;
  boost::uint32_t numVertices = boost::num_vertices(p_graph);

  for (; i < p_cats.size(); ++i) {
    nextV = find(p_cats[i], p_graph, curV);

    // break, because we have to add nodes now
    if (nextV == NULL) {
      break;
    } else {
      curV = nextV;
    }
  }

  // add the nodes
  for (; i < p_cats.size(); ++i) {
    Vertex newV = boost::add_vertex(p_graph);
    std::pair<Edge, bool> e = add_edge(curV, newV, p_graph);
    itypes::StringVector tokens;
    boost::split(tokens, p_cats[i], boost::is_any_of(":"), boost::token_compress_on);

    if (tokens.size() == 1) {
      p_graph[newV].level = i+1.0;
      p_graph[newV].name = tokens[0];
    } else {
      p_graph[newV].level = boost::lexical_cast<double>(tokens[1]);
      p_graph[newV].name = tokens[0];
    }
    p_graph[newV].id = numVertices;
    curV = newV;
    numVertices += 1;
  }
}

template <class StringVector, class DoubleVector>
class dfs_euler_circuit : public boost::default_dfs_visitor
{
 public:
  dfs_euler_circuit(StringVector &p_euler_circuit, DoubleVector &p_levels)
      : m_euler_circuit(p_euler_circuit), m_levels(p_levels) {
#ifndef NDEBUG
    std::cout << "dfs_euler_circuit" << std::endl;
#endif /* NDEBUG */
  }

  ~dfs_euler_circuit() {
#ifndef NDEBUG
    std::cout << "~dfs_euler_circuit" << std::endl;
    std::copy(m_euler_circuit.begin(), m_euler_circuit.end(), std::ostream_iterator<std::string>(std::cout, " "));
    std::cout << std::endl;
    std::copy(m_levels.begin(), m_levels.end(), std::ostream_iterator<double>(std::cout, " "));
    std::cout << std::endl;
#endif /* NDEBUG */
  }

  template <typename Vertex, typename Graph>
  void initialize_vertex(Vertex s, Graph &g) {
#ifndef NDEBUG
    std::cout << "Initialise: " << g[s].name << std::endl;
#endif /* NDEBUG */
  }

  template <typename Vertex, typename Graph>
  void start_vertex(Vertex s, Graph &g) {
#ifndef NDEBUG
    std::cout << "Start: " << g[s].name << std::endl;
#endif /* NDEBUG */
  }

  template <typename Vertex, typename Graph>
  void discover_vertex(Vertex u, Graph &g) {
#ifndef NDEBUG
    std::cout << "Discover: " << g[u].name << std::endl;
#endif /* NDEBUG */
    m_euler_circuit.push_back(g[u].name);
    m_levels.push_back(g[u].level);
  }

  template <typename Edge, typename Graph>
  void examine_edge(Edge e, Graph &g) {
#ifndef NDEBUG
    std::cout << "Examine: " << g[boost::source(e, g)].name << "-" << g[boost::target(e, g)].name << std::endl;
#endif /* NDEBUG */
  }

  template <typename Edge, typename Graph>
  void tree_edge(Edge e, Graph &g) {
#ifndef NDEBUG
    std::cout << "Tree: " << g[boost::source(e, g)].name << "-" << g[boost::target(e, g)].name << std::endl;
#endif /* NDEBUG */
  }

  template <typename Edge, typename Graph>
  void back_edge(Edge e, Graph &g) {
#ifndef NDEBUG
    std::cout << "Back: " << g[boost::source(e, g)].name << "-" << g[boost::target(e, g)].name << std::endl;
#endif /* NDEBUG */
  }

  template <typename Edge, typename Graph>
  void forward_or_cross_edge(Edge e, Graph &g) {
#ifndef NDEBUG
    std::cout << "Forward or Cross: " << g[boost::source(e, g)].name << "-" << g[boost::target(e, g)].name << std::endl;
#endif /* NDEBUG */
  }

  template <typename Edge, typename Graph>
  void finish_edge(Edge e, Graph &g) {
#ifndef NDEBUG
    std::cout << "Finish: " << g[boost::source(e, g)].name << "-" << g[boost::target(e, g)].name << std::endl;
#endif /* NDEBUG */
  }

  template <typename Vertex, typename Graph>
  void finish_vertex(Vertex u, Graph &g) {
#ifndef NDEBUG
    std::cout << "Finish: " << g[u].name << std::endl;
#endif /* NDEBUG */

    // we know in a tree there is only one parent
    InEdgeIterator in_begin, in_end;
    for (boost::tie(in_begin, in_end) = boost::in_edges(u, g); in_begin != in_end; ++in_begin) {
      m_euler_circuit.push_back(g[boost::source(*in_begin, g)].name);
      m_levels.push_back(g[boost::source(*in_begin, g)].level);
    }
  }

 private:
  StringVector &m_euler_circuit;
  DoubleVector &m_levels;
};


}


#endif
