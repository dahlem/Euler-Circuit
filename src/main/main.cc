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

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/cstdint.hpp>
#include <boost/tokenizer.hpp>

#include "CL.hh"
namespace imain = icd9::main;

#include "Graph.hh"
namespace igraph = icd9::graph;

#include "Types.hh"
namespace itypes = icd9::types;

typedef boost::tokenizer <boost::escaped_list_separator <char> > Tokenizer;


int main(int argc, char *argv[])
{
  imain::args_t args;
  imain::CL cl;

  if (cl.parse(argc, argv, args)) {
    return EXIT_SUCCESS;
  }

  std::ifstream icd9File;

  icd9File.open(args.icd9Codes.c_str());
  if (!icd9File.is_open()) {
    std::cerr << "Could not open file: " << args.icd9Codes << std::endl;
    return EXIT_FAILURE;
  }

#ifndef NDEBUG
  std::cout << "Reading ICD-9 codes..." << std::endl;
#endif /* NDEBUG */

  igraph::Graph g;

  // the ICD-9 tree is disjointed if we were not to introduce an artificial root vertex
  igraph::Vertex root = boost::add_vertex(g);
  g[root].level = 0;
  g[root].name = "epsilon"; // root vertex name

  std::string line;
  while (!icd9File.eof()) {
    std::getline(icd9File, line);
#ifndef NDEBUG
    std::cout << "Read line: " << line << std::endl;
#endif /* NDEBUG */

    if (line != "") {
      itypes::ICD9Categories cats;
      boost::split(cats, line, boost::is_any_of(","), boost::token_compress_on);

      // build the tree structure
      igraph::add(cats, g, root);
    }
  }
  icd9File.close();

  igraph::ColorPropertyMap vertex_color_map = boost::get(&igraph::VertexProperties::color, g);
  boost::depth_first_search(g, boost::visitor(igraph::dfs_euler_circuit()));

  return EXIT_SUCCESS;
}
