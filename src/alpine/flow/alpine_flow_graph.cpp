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
/// file: alpine_flow_graph.cpp
///
//-----------------------------------------------------------------------------

#include "alpine_flow_graph.hpp"

// standard lib includes
#include <iostream>
#include <string.h>
#include <limits.h>
#include <cstdlib>

//-----------------------------------------------------------------------------
// thirdparty includes
//-----------------------------------------------------------------------------

// conduit includes
#include <conduit.hpp>


//-----------------------------------------------------------------------------
// alpine includes
//-----------------------------------------------------------------------------
#include <alpine_logging.hpp>


using namespace conduit;
using namespace std;

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

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
Graph::Graph(Workspace *w)
:m_workspace(w),
 m_filter_count(0)
{
    
}

//-----------------------------------------------------------------------------
Graph::~Graph()
{
    reset();
}


//-----------------------------------------------------------------------------
Workspace &
Graph::workspace()
{
    return *m_workspace;
}

//-----------------------------------------------------------------------------
void 
Graph::reset()
{
    // delete all filters

    std::map<std::string,Filter*>::iterator itr;
    for(itr = m_filters.begin(); itr != m_filters.end(); itr++)
    {
        delete itr->second;
    }

    m_filters.clear();
    
    // // delete all registered types
    // for(itr = m_filter_types.begin(); itr != m_filter_types.end(); itr++)
    // {
    //     delete itr->second;
    // }

    m_filter_types.clear();
    
    m_edges.reset();

}

//-----------------------------------------------------------------------------
bool
Graph::has_registered_filter_type(const std::string &name)
{
    std::map<std::string,FilterType>::iterator itr = m_filter_types.find(name);
    return itr != m_filter_types.end();
}

//-----------------------------------------------------------------------------
void
Graph::register_filter_type(FilterType fr)
{
    // check that filter is valid by creating
    // an instance
    
    Filter *f = fr();
    Node p;
    f->init(this,"",p);
    
    std::string f_type_name =f->type_name();
    
    // we no longer need this instance ...
    delete f;
    
    if(has_registered_filter_type(f_type_name))
    {
        ALPINE_WARN("filter type named:"
                     << f_type_name 
                    << " is already registered");
        return;
    }
    
    m_filter_types[f_type_name] = fr;

}


//-----------------------------------------------------------------------------
Filter *
Graph::add_filter(const std::string &filter_type,
                  const std::string &filter_name)
{
    Node filter_params;
    return add_filter(filter_type, filter_name, filter_params);
}


//-----------------------------------------------------------------------------
Filter *
Graph::add_filter(const std::string &filter_type,
                  const std::string &filter_name,
                  const Node &filter_params)
{
    if(has_filter(filter_name))
    {
        ALPINE_WARN("Cannot create filter, filter named: " << filter_name
                     << " already exists in Graph");
        return NULL;
    }

    std::map<std::string,FilterType>::iterator itr = m_filter_types.find(filter_type);
    if(itr == m_filter_types.end())
    {
        ALPINE_WARN("Cannot create unknown filter type: "
                    << filter_type);
        return NULL;
    }
    
    // this creates a new instance ...
    Filter *f =itr->second();
    
    f->init(this,
            filter_name,
            filter_params);
    
    m_filters[filter_name] = f;
    
    NodeConstIterator ports_itr(&f->port_names());

    while(ports_itr.has_next())
    {
        std::string port_name = ports_itr.next().as_string();
        m_edges["in"][filter_name][port_name] = DataType::empty();
    }
    
    if(f->output_port())
    {
        m_edges["out"][filter_name] = DataType::list();
    }
    
    m_filter_count++;
    
    return f;
}

//-----------------------------------------------------------------------------
Filter *
Graph::add_filter(const std::string &filter_type)
{
    ostringstream oss;
    oss << "f_" << m_filter_count;
    Node filter_params;
    return add_filter(filter_type, oss.str(), filter_params);
}

//-----------------------------------------------------------------------------
Filter *
Graph::add_filter(const std::string &filter_type,
                  const Node &filter_params)
{
    ostringstream oss;
    oss << "f_" << m_filter_count;
    return add_filter(filter_type, oss.str(), filter_params);
}



//-----------------------------------------------------------------------------
void 
Graph::connect(const std::string &src_name,
                     const std::string &des_name,
                     const std::string &port_name)
{
    // make sure we have a filter with the given name
    
    if(!has_filter(src_name))
    {
        ALPINE_WARN("source filter named: " << src_name
                    << "does not exist in FilterGraph");
        return;
    }

    if(!has_filter(des_name))
    {
        ALPINE_WARN("destination filter named: " << des_name
                    << "does not exist in FilterGraph");
        return;
    }


    Filter *src_filter = m_filters[src_name];
    Filter *des_filter = m_filters[des_name];

    // make sure it has an input port with the given name
    if(!des_filter->has_port(port_name))
    {
        ALPINE_WARN("destination filter: "
                     << des_name 
                     << " (type: " << des_filter->type_name() << ")"
                     << "does not have input port named:"
                     << port_name);
        return;
    }
    
    m_edges["in"][des_name][port_name] = src_name;
    m_edges["out"][src_name].append().set(des_name);
}



//-----------------------------------------------------------------------------
bool
Graph::has_filter(const std::string &name)
{
    std::map<std::string,Filter*>::iterator itr = m_filters.find(name);
    return itr != m_filters.end();
}

//-----------------------------------------------------------------------------
void
Graph::remove_filter(const std::string &name)
{
    if(!has_filter(name))
    {
        ALPINE_WARN("filter named: " << name
                     << "does not exist in FilterGraph");
        return;
    }

    // remove from m_filters, and prune edges
    std::map<std::string,Filter*>::iterator itr = m_filters.find(name);
    
    delete itr->second;
    
    m_filters.erase(itr);
    
    m_edges["in"].remove(name);
    m_edges["out"].remove(name);
}

//-----------------------------------------------------------------------------
const Node &
Graph::edges() const
{
    return m_edges;
}

//-----------------------------------------------------------------------------
const Node &
Graph::edges_out(const std::string &f_name) const
{
    return m_edges["out"][f_name];
}

//-----------------------------------------------------------------------------
const Node &
Graph::edges_in(const std::string &f_name) const
{
    return m_edges["in"][f_name];
}


//-----------------------------------------------------------------------------
std::map<std::string,Filter*>  &
Graph::filters()
{
    return m_filters;
}



//-----------------------------------------------------------------------------
void
Graph::info(Node &out)
{
    out.reset();
    Node &filts = out["filters"];
    
    std::map<std::string,Filter*>::iterator itr;
    for(itr = m_filters.begin(); itr != m_filters.end(); itr++)
    {
        itr->second->info(filts[itr->first]);
    }

    out["edges"] = m_edges;

}


//-----------------------------------------------------------------------------
std::string
Graph::to_json()
{
    Node out;
    info(out);
    ostringstream oss;
    out.to_json_stream(oss);
    return oss.str();
}

//-----------------------------------------------------------------------------
void
Graph::print()
{
    ALPINE_INFO(to_json());
}


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



