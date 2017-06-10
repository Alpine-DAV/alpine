//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2015-2017, Lawrence Livermore National Security, LLC.
// 
// Produced at the Lawrence Livermore National Laboratory
// 
// LLNL-CODE-716457
// 
// All rights reserved.
// 
// This file is part of Alpine. 
// 
// For details, see: http://software.llnl.gov/alpine/.
// 
// Please also read alpine/LICENSE
// 
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:
// 
// * Redistributions of source code must retain the above copyright notice, 
//   this list of conditions and the disclaimer below.
// 
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the disclaimer (as noted below) in the
//   documentation and/or other materials provided with the distribution.
// 
// * Neither the name of the LLNS/LLNL nor the names of its contributors may
//   be used to endorse or promote products derived from this software without
//   specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL LAWRENCE LIVERMORE NATIONAL SECURITY,
// LLC, THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
// DAMAGES  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
// IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
// POSSIBILITY OF SUCH DAMAGE.
// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//


//-----------------------------------------------------------------------------
///
/// file: alpine_flow_graph.hpp
///
//-----------------------------------------------------------------------------

#ifndef ALPINE_FLOW_GRAPH_HPP
#define ALPINE_FLOW_GRAPH_HPP

#include <alpine_flow_filter.hpp>


//-----------------------------------------------------------------------------
// -- begin alpine:: --
//-----------------------------------------------------------------------------
namespace alpine
{

//-----------------------------------------------------------------------------
// -- begin alpine::flow --
//-----------------------------------------------------------------------------
namespace flow
{

class Workspace;

//-----------------------------------------------------------------------------
///
/// Filter Graph
///
//-----------------------------------------------------------------------------

class Graph
{
public:
    
    friend class Workspace;
    friend class ExecutionPlan;
    
   ~Graph();

    
    // basic factory
    bool has_registered_filter_type(const std::string &filter_type);
    void register_filter_type(FilterType fr);

    Workspace &workspace();


    // interface to construct a graph
    Filter *add_filter(const std::string &filter_type,
                       const std::string &name);


    Filter *add_filter(const std::string &filter_type,
                       const std::string &name,
                       const conduit::Node &params);

    // let the graph gen a unique a name
    Filter *add_filter(const std::string &filter_type);

    // let the graph gen a unique a name
    Filter *add_filter(const std::string &filter_type,
                       const conduit::Node &params);


    void connect(const std::string &src_name,
                 const std::string &des_name,
                 const std::string &port_name);

    void connect(const std::string &src_name,
                 const std::string &des_name,
                 int port_idx);


    bool has_filter(const std::string &name);

    void remove_filter(const std::string &name);

    void reset();

    /// human friendly output
    void        info(conduit::Node &out);
    std::string to_json();
    void        print();


private:
    Graph(Workspace *w);

    const conduit::Node &edges()  const;
    const conduit::Node &edges_in(const std::string &f_name)  const;
    const conduit::Node &edges_out(const std::string &f_name) const;

    std::map<std::string,Filter*> &filters();


    Workspace                       *m_workspace;
    conduit::Node                    m_edges;
    std::map<std::string,Filter*>    m_filters;
    std::map<std::string,FilterType> m_filter_types;
    
    int                              m_filter_count;

};


//-----------------------------------------------------------------------------
};
//-----------------------------------------------------------------------------
// -- end alpine::flow --
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
};
//-----------------------------------------------------------------------------
// -- end alpine:: --
//-----------------------------------------------------------------------------

#endif
//-----------------------------------------------------------------------------
// -- end header ifdef guard
//-----------------------------------------------------------------------------


