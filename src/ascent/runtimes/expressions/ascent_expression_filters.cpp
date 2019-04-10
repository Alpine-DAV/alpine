//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2015-2019, Lawrence Livermore National Security, LLC.
//
// Produced at the Lawrence Livermore National Laboratory
//
// LLNL-CODE-716457
//
// All rights reserved.
//
// This file is part of Ascent.
//
// For details, see: http://ascent.readthedocs.io/.
//
// Please also read ascent/LICENSE
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
/// file: ascent_expression_filters.cpp
///
//-----------------------------------------------------------------------------

#include "ascent_expression_filters.hpp"

//-----------------------------------------------------------------------------
// thirdparty includes
//-----------------------------------------------------------------------------

// conduit includes
#include <conduit.hpp>

//-----------------------------------------------------------------------------
// ascent includes
//-----------------------------------------------------------------------------
#include <ascent_logging.hpp>
#include "ascent_conduit_reductions.hpp"
#include "ascent_blueprint_architect.hpp"
#include <flow_graph.hpp>
#include <flow_workspace.hpp>

#include <limits>

#ifdef ASCENT_MPI_ENABLED
#include <mpi.h>
#endif
using namespace conduit;
using namespace std;

using namespace flow;

//-----------------------------------------------------------------------------
// -- begin ascent:: --
//-----------------------------------------------------------------------------
namespace ascent
{

//-----------------------------------------------------------------------------
// -- begin ascent::runtime --
//-----------------------------------------------------------------------------
namespace runtime
{

//-----------------------------------------------------------------------------
// -- begin ascent::runtime::expressions--
//-----------------------------------------------------------------------------
namespace expressions
{

namespace detail
{

bool is_math(const std::string &op)
{
  if(op == "*" || op == "+" || op == "/" || op == "-")
  {
    return true;
  }
  else
  {
    return false;
  }
}

template<typename T>
T math_op(const T& lhs, const T& rhs, const std::string &op)
{
  T res;
  if(op == "+")
  {
    res = lhs + rhs;
  }
  else if(op == "-")
  {
    res = lhs - rhs;
  }
  else if(op == "*")
  {
    res = lhs * rhs;
  }
  else if(op == "/")
  {
    res = lhs / rhs;
  }
  else
  {
    ASCENT_ERROR("unknow math op "<<op);
  }
  return res;
}

template<typename T>
int comp_op(const T& lhs, const T& rhs, const std::string &op)
{
  int res;
  if(op == "<")
  {
    res = lhs < rhs;
  }
  else if(op == "<=")
  {
    res = lhs <= rhs;
  }
  else if(op == ">")
  {
    res = lhs > rhs;
  }
  else if(op == ">=")
  {
    res = lhs >= rhs;
  }
  else
  {
    ASCENT_ERROR("unknow comparison op "<<op);
  }
  return res;
}

} // namespace detail

//-----------------------------------------------------------------------------
Integer::Integer()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
Integer::~Integer()
{
// empty
}

//-----------------------------------------------------------------------------
void
Integer::declare_interface(Node &i)
{
    i["type_name"]   = "expr_integer";
    i["port_names"] = DataType::empty();
    i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
Integer::verify_params(const conduit::Node &params,
                               conduit::Node &info)
{
    info.reset();
    bool res = true;
    if(!params.has_path("value"))
    {
       info["errors"].append() = "Missing required numeric parameter 'value'";
       res = false;
    }
    return res;
}


//-----------------------------------------------------------------------------
void
Integer::execute()
{

   conduit::Node *output = new conduit::Node();
   (*output)["value"] = params()["value"].to_float64();
   (*output)["type"] = "numeric";
   set_output<conduit::Node>(output);
}

//-----------------------------------------------------------------------------
Double::Double()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
Double::~Double()
{
// empty
}

//-----------------------------------------------------------------------------
void
Double::declare_interface(Node &i)
{
    i["type_name"]   = "expr_double";
    i["port_names"] = DataType::empty();
    i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
Double::verify_params(const conduit::Node &params,
                      conduit::Node &info)
{
    info.reset();
    bool res = true;
    if(!params.has_path("value"))
    {
       info["errors"].append() = "Missing required numeric parameter 'value'";
       res = false;
    }
    return res;
}


//-----------------------------------------------------------------------------
void
Double::execute()
{

   conduit::Node *output = new conduit::Node();
   (*output)["value"] = params()["value"].to_float64();
   (*output)["type"] = "numeric";
   set_output<conduit::Node>(output);
}

//-----------------------------------------------------------------------------
MeshVar::MeshVar()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
MeshVar::~MeshVar()
{
// empty
}

//-----------------------------------------------------------------------------
void
MeshVar::declare_interface(Node &i)
{
    i["type_name"]   = "expr_meshvar";
    i["port_names"] = DataType::empty();
    i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
MeshVar::verify_params(const conduit::Node &params,
                       conduit::Node &info)
{
    info.reset();
    bool res = true;
    if(!params.has_path("value"))
    {
       info["errors"].append() = "Missing required string parameter 'value'";
       res = false;
    }
    return res;
}

//-----------------------------------------------------------------------------
void
MeshVar::execute()
{
   conduit::Node *output = new conduit::Node();

   (*output)["value"] = params()["value"].as_string();
   (*output)["type"] = "meshvar";
   set_output<conduit::Node>(output);
}

//-----------------------------------------------------------------------------
BinaryOp::BinaryOp()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
BinaryOp::~BinaryOp()
{
// empty
}

//-----------------------------------------------------------------------------
void
BinaryOp::declare_interface(Node &i)
{
    i["type_name"]   = "expr_binary_op";
    i["port_names"].append() = "lhs";
    i["port_names"].append() = "rhs";
    i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
BinaryOp::verify_params(const conduit::Node &params,
                        conduit::Node &info)
{
    info.reset();
    bool res = true;
    if(!params.has_path("op_string"))
    {
       info["errors"].append() = "Missing required string parameter 'op_string'";
       res = false;
    }
    return res;
}


//-----------------------------------------------------------------------------
void
BinaryOp::execute()
{

  Node *n_lhs = input<Node>("lhs");
  Node *n_rhs = input<Node>("rhs");

  const Node &lhs = (*n_lhs)["value"];
  const Node &rhs = (*n_rhs)["value"];


  n_lhs->print();
  n_rhs->print();

  bool has_float = false;

  if(lhs.dtype().is_floating_point() ||
     rhs.dtype().is_floating_point())
  {
    has_float = true;
  }

  conduit::Node *output = new conduit::Node();

  std::string op = params()["op_string"].as_string();
  // promote to double if at one is a double
  bool is_math = detail::is_math(op);

  if(has_float)
  {
    double d_rhs = rhs.to_float64();
    double d_lhs = lhs.to_float64();
    if(is_math)
    {
      (*output)["value"] = detail::math_op(d_lhs, d_rhs, op);
    }
    else
    {
      (*output)["value"] = detail::comp_op(d_lhs, d_rhs, op);
    }
    (*output)["type"] = "numeric";
  }
  else
  {
    int i_rhs = rhs.to_int32();
    int i_lhs = lhs.to_int32();
    if(is_math)
    {
      (*output)["value"] = detail::math_op(i_lhs, i_rhs, op);
    }
    else
    {
      (*output)["value"] = detail::comp_op(i_lhs, i_rhs, op);
    }
    (*output)["type"] = "boolean";
  }

  std::cout<<" operation "<<op<<"\n";

  set_output<conduit::Node>(output);
}

//-----------------------------------------------------------------------------
ScalarMax::ScalarMax()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
ScalarMax::~ScalarMax()
{
// empty
}

//-----------------------------------------------------------------------------
void
ScalarMax::declare_interface(Node &i)
{
    i["type_name"]   = "scalar_max";
    i["port_names"].append() = "arg1";
    i["port_names"].append() = "arg2";
    i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
ScalarMax::verify_params(const conduit::Node &params,
                        conduit::Node &info)
{
    info.reset();
    bool res = true;
    return res;
}


//-----------------------------------------------------------------------------
void
ScalarMax::execute()

{

  Node *arg1 = input<Node>("arg1");
  Node *arg2 = input<Node>("arg2");


  arg1->print();
  arg2->print();

  bool has_float = false;

  if(arg1->dtype().is_floating_point() ||
     arg2->dtype().is_floating_point())
  {
    has_float = true;
  }

  conduit::Node *output = new conduit::Node();

  if(has_float)
  {
    double d_rhs = arg1->to_float64();
    double d_lhs = arg2->to_float64();
    (*output)["value"] = std::max(d_lhs, d_rhs);
  }
  else
  {
    int i_rhs = arg1->to_int32();
    int i_lhs = arg2->to_int32();
    (*output)["value"] = std::max(i_lhs, i_rhs);
  }

  (*output)["type"] = "numeric";
  set_output<conduit::Node>(output);
}

//-----------------------------------------------------------------------------
FieldMax::FieldMax()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
FieldMax::~FieldMax()
{
// empty
}

//-----------------------------------------------------------------------------
void
FieldMax::declare_interface(Node &i)
{
    i["type_name"]   = "field_max";
    i["port_names"].append() = "arg1";
    i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
FieldMax::verify_params(const conduit::Node &params,
                        conduit::Node &info)
{
    info.reset();
    bool res = true;
    return res;
}


//-----------------------------------------------------------------------------
void
FieldMax::execute()

{

  Node *arg1 = input<Node>("arg1");

  arg1->print();

  const std::string field = (*arg1)["value"].as_string();

  conduit::Node *output = new conduit::Node();

  if(!graph().workspace().registry().has_entry("dataset"))
  {
    ASCENT_ERROR("FieldMax: Missing dataset");
  }

  conduit::Node *dataset = graph().workspace().registry().fetch<Node>("dataset");

  //dataset->print();
  bool has_field = false;
  bool is_scalar = false;
  for(int i = 0; i < dataset->number_of_children(); ++i)
  {
    const conduit::Node &dom = dataset->child(i);
    if(!has_field && dom.has_path("fields/"+field))
    {
      has_field = true;
      const conduit::Node &n_field = dom["fields/"+field];
      const int num_children = n_field["values"].number_of_children();
      if(num_children == 0)
      {
        is_scalar = true;
      }
    }
  }

  double max_val;

  if(!has_field)
  {
    std::vector<std::string> names = dataset->child(0)["fields"].child_names();
    std::stringstream ss;
    ss<<"[";
    for(int i = 0; i < names.size(); ++i)
    {
      ss<<" "<<names[i];
    }
    ss<<"]";
    ASCENT_ERROR("FieldMax: dataset does not contain field '"<<field<<"'"
                 <<" known = "<<ss.str());
  }

  if(!is_scalar)
  {
    ASCENT_ERROR("FieldMax: field '"<<field<<"' is not a scalar");
  }

  const std::string assoc_str = dataset->child(0)["fields/" + field + "/association"].as_string();

  double max_value = std::numeric_limits<double>::lowest();

  std::cout<<"init max "<<max_value<<"\n";

  int domain = -1;
  int index = -1;

  for(int i = 0; i < dataset->number_of_children(); ++i)
  {
    const conduit::Node &dom = dataset->child(i);
    if(dom.has_path("fields/"+field))
    {
      const std::string path = "fields/" + field + "/values";
      conduit::Node res;
      res = array_max(dom[path]);
      res.print();
      double a_max = res["value"].to_float64();
      std::cout<<"max "<<max_value<<"  current "<<a_max<<"\n";
      if(a_max > max_value)
      {
        max_value = a_max;
        index = res["index"].as_int32();
        domain = i;
      }
    }
  }

  std::cout<<"max value "<<max_value<<"\n";
  std::cout<<"index "<<index<<"\n";

  conduit::Node loc;
  if(assoc_str == "vertex")
  {
    loc = point_location(dataset->child(domain),index);
  }
  else if(assoc_str == "element")
  {
    loc = cell_location(dataset->child(domain),index);
  }
  else
  {
    ASCENT_ERROR("Location for "<<assoc_str<<" not implemented");
  }

  loc.print();

  int rank = 0;
#ifdef ASCENT_MPI_ENABLED
  struct MaxLoc
  {
    double value;
    int rank;
  };

  MPI_Comm mpi_comm = MPI_Comm_f2c(Workspace::default_mpi_comm());
  MPI_Comm_rank(mpi_comm, &rank);

  MaxLoc maxloc = {max_value, rank};
  MaxLoc maxloc_res;
  MPI_Allreduce( &maxloc, &maxloc_res, 1, MPI_DOUBLE_INT, MPI_MAXLOC, mpi_comm);
  double * ploc = loc.as_float64_ptr();
  MPI_Bcast(ploc, 3, MPI_DOUBLE, maxloc_res.rank, mpi_comm);
  loc.set(ploc, 3);
  if(rank == 0)
  {
    std::cout<<"Winning rank = "<<maxloc_res.rank<<"\n";
    std::cout<<"loc ";
    loc.print();
  }
#endif

  (*output)["value"] = max_value;
  (*output)["type"] = "numeric";
  (*output)["atts/position"] = loc;

  set_output<conduit::Node>(output);
}


//-----------------------------------------------------------------------------
Position::Position()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
Position::~Position()
{
// empty
}

//-----------------------------------------------------------------------------
void
Position::declare_interface(Node &i)
{
    i["type_name"]   = "expr_position";
    i["port_names"].append() = "arg1";
    i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
Position::verify_params(const conduit::Node &params,
                        conduit::Node &info)
{
    info.reset();
    bool res = true;
    return res;
}


//-----------------------------------------------------------------------------
void
Position::execute()
{

  Node *n_in = input<Node>("arg1");

  if(!n_in->has_path("atts/position"))
  {
    ASCENT_ERROR("Position: input does not have 'position' attribute");
  }

  conduit::Node *output = new conduit::Node();

  (*output)["value"] = (*n_in)["atts/position"];
  (*output)["type"] = "vector";

  set_output<conduit::Node>(output);
}

//-----------------------------------------------------------------------------
};
//-----------------------------------------------------------------------------
// -- end ascent::runtime::expressions--
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
};
//-----------------------------------------------------------------------------
// -- end ascent::runtime --
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
};
//-----------------------------------------------------------------------------
// -- end ascent:: --
//-----------------------------------------------------------------------------





