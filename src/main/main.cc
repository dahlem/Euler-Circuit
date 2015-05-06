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

#if HAVE_CONFIG_H
# include <config.h>
#endif

#ifndef __STDC_CONSTANT_MACROS
# define __STDC_CONSTANT_MACROS
#endif /* __STDC_CONSTANT_MACROS */

#include <fstream>
#include <iostream>
#include <vector>

#include <boost/cstdint.hpp>
#include <boost/tokenizer.hpp>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <boost/graph/graphml.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/undirected_dfs.hpp>

#include "CL.hh"

#include "Graph.hh"
namespace igraph = graph;

#include "Types.hh"
namespace itypes = types;

typedef boost::tokenizer <boost::escaped_list_separator <char> > Tokenizer;


int main(int argc, char *argv[])
{
  args_t args;
  CL cl;

  if (cl.parse(argc, argv, args)) {
    return EXIT_SUCCESS;
  }

  std::ifstream codesFile;

  codesFile.open(args.codes.c_str());
  if (!codesFile.is_open()) {
    std::cerr << "Could not open file: " << args.codes << std::endl;
    return EXIT_FAILURE;
  }

#ifndef NDEBUG
  std::cout << "Reading codes..." << std::endl;
#endif /* NDEBUG */

  igraph::Graph g;

  // the ICD-9 tree is disjointed if we were not to introduce an artificial root vertex
  igraph::Vertex root = boost::add_vertex(g);
  g[root].level = 0;
  g[root].id = 1;
  g[root].name = "epsilon"; // root vertex name

  std::string line;
  while (!codesFile.eof()) {
    std::getline(codesFile, line);
#ifndef NDEBUG
    std::cout << "Read line: " << line << std::endl;
#endif /* NDEBUG */

    if (line != "") {
      itypes::StringVector cats;
      boost::split(cats, line, boost::is_any_of(","), boost::token_compress_on);

      // build the tree structure
      igraph::add(cats, g, root);
    }
  }
  codesFile.close();

  itypes::StringVector euler_circuit;
  itypes::IntVector levels;
  igraph::dfs_euler_circuit<itypes::StringVector, itypes::IntVector> dfs_euler(euler_circuit, levels);
  boost::depth_first_search(g, boost::visitor(dfs_euler));

  // serialise the ICD-9 tree, euler circuit and the corresponding levels
  std::string outEulerFile = args.results_dir + "/euler_circuit.dat";
  std::ofstream outEuler(outEulerFile.c_str(), std::ios::out);
  std::copy(euler_circuit.begin(), euler_circuit.end(), std::ostream_iterator<std::string>(outEuler, "\n"));
  outEuler.close();

  std::string outLevelsFile = args.results_dir + "/levels.dat";
  std::ofstream outLevels(outLevelsFile.c_str(), std::ios::out);
  std::copy(levels.begin(), levels.end(), std::ostream_iterator<boost::uint32_t>(outLevels, "\n"));
  outLevels.close();

  boost::dynamic_properties dp;
  dp.property("id", get(&igraph::VertexProperties::id, g));
  dp.property("Name", get(&igraph::VertexProperties::name, g));
  dp.property("Level", get(&igraph::VertexProperties::level, g));

  std::string outTreeFile = args.results_dir + "/tree.gml";
  std::ofstream outTree(outTreeFile.c_str(), std::ios::out);
  boost::write_graphml(outTree, g, get(&igraph::VertexProperties::id, g), dp, false);
  outTree.close();

  std::string outGVFile = args.results_dir + "/tree.dot";
  std::ofstream outGV(outGVFile.c_str(), std::ios::out);
  boost::write_graphviz(outGV, g, boost::make_label_writer(get(&igraph::VertexProperties::name, g)));
  outGV.close();

  return EXIT_SUCCESS;
}
