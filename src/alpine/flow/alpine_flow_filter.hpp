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
/// file: alpine_flow_filter.hpp
///
//-----------------------------------------------------------------------------

#ifndef ALPINE_FLOW_FILTER_HPP
#define ALPINE_FLOW_FILTER_HPP

#include <conduit.hpp>

#include <alpine_flow_data.hpp>
#include <alpine_flow_registry.hpp>


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
class Graph;

//-----------------------------------------------------------------------------
///
/// Filter Interface
///
///
/// Filters optionally provide:
///   - A set of named input ports
///   - output
///   - A set of default parameters 
///
///  To create a new filter, create a new subclass of Filter and:
///
///  1) Declare the filter interrace, by adding the following entires to the 
//      interface() node in in your constructor.
///
///  MyFilter::MyFilter
///  :Filter()
///  {
///    Node &i = interface();
///
///    // unique filter name
///    i["type_name"]   = "my_filter";
///
///    // declare if this filter provides output
///    i["output_port"] = {"true" | "false"};
///
///    // declare the names of this filters input ports
///    // Provide a conduit list of strings with the names of the input ports 
///    // or DataType::empty() if there are no input ports.
///    i["port_names"].append().set("in");
///
///    // Set any default parameters.
///    // default_params can be any conduit tree, params() will be
///    // inited with a *copy* of the default_params when the filter is
///    // added to the filter graph.
///    i["default_params"]["inc"].set((int)1);
///  }
///
///  2) Implement an execute() method:
///
///  void MyFilter::execute()
///  {
///     // If your filter has input ports, input data can be fetched by name
///     Node &in_0 = input("in");
///     // or index:
///     Node &in_0 = input(0);
///  
///     // You can access filter parameters via params()
///     int val = params()["my_knob"].value();
///
///     // If your filter provides output, set your output data:
///     Node *my_result = new Node();
///     output()->set(Data(my_result); 
///     // the registry manages result lifetimes.
///
///  }
/// 
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
class Filter
{
public:
    
    friend class Graph;
    friend class Workspace;
    
    virtual ~Filter();


    // implement these:

    /// override to imp filter's work
    virtual void          execute() = 0;


    /// override to 
    /// (used as a guard when a filter instance is created in a graph)
    virtual bool          verify_params(const conduit::Node &params,
                                        conduit::Node &info);


    // filter interface
    
    /// static method that checks if conduit node passed conforms to what 
    /// is needed to declare a filter interface.
    /// (used as a guard when a filter type is added to a graph)
    static bool           verify_interface(const conduit::Node &i,
                                           conduit::Node &info);
  
    std::string           type_name()   const;
    const conduit::Node  &port_names()  const;
    bool                  output_port() const;
    
    const conduit::Node  &default_params() const;

    int                   number_of_input_ports() const;
    bool                  has_port(const std::string &name) const;
    std::string           port_index_to_name(int idx) const;

    // instance properties 
    std::string           name() const;
    std::string           detailed_name() const;


    // methods used to implement filter interface
    conduit::Node         &interface();


    // methods used to implement filter exec
    conduit::Node         &params();

    Data                  &input(const std::string &port_name);
    Data                  &input(int idx);
    Data                  &output();
    
    Graph                 &graph();

    // methods used to help build a filter graph 
    // graph().connect(f->name(),this->name(),port_name);
    void                  connect_input_port(const std::string &port_name,
                                             Filter *filter);

    void                  connect_input_port(int idx,
                                             Filter *filter);

    
    /// human friendly output
    void                   info(conduit::Node &out);
    std::string            to_json();
    void                   print();

protected:
    Filter();

private:

    // used by ws interface to imp data flow exec
    void                    set_input(const std::string &port_name,
                                      Data ds);

    void                    init(Graph *graph,
                                 const std::string &name,
                                 const conduit::Node &params);

    void                    reset_inputs_and_output();


    conduit::Node           &properties();
    const conduit::Node     &properties() const;

    
    Graph                         *m_graph;
    
    conduit::Node                  m_props;
    Data                           m_out;
    std::map<std::string,Data>     m_inputs;

};

//-----------------------------------------------------------------------------
typedef Filter *(*FilterType)();


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


