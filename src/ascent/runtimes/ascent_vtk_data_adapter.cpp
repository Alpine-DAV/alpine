//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2015-2018, Lawrence Livermore National Security, LLC.
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
/// file: ascent_data_adapter.cpp
///
//-----------------------------------------------------------------------------
#include "ascent_vtk_data_adapter.hpp"

// standard lib includes
#include <iostream>
#include <string.h>
#include <limits.h>
#include <cstdlib>
#include <sstream>
#include <type_traits>

// third party includes

// mpi
#ifdef ASCENT_MPI_ENABLED
#include <mpi.h>
#endif

// VTK includes
#include "vtkCellArray.h"
#include "vtkDoubleArray.h"
#include "vtkInformationDoubleKey.h"
#include "vtkIntArray.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkCellData.h"
#include "vtkFloatArray.h"
#include "vtkDoubleArray.h"
#include "vtkImageData.h"
#include "vtkRectilinearGrid.h"
#include "vtkStructuredGrid.h"
#include "vtkUnstructuredGrid.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkSOADataArrayTemplate.h"

// other ascent includes
#include <ascent_logging.hpp>
#include <ascent_block_timer.hpp>

#include <conduit_blueprint.hpp>

using namespace std;
using namespace conduit;

//-----------------------------------------------------------------------------
// -- begin ascent:: --
//-----------------------------------------------------------------------------
namespace ascent
{

//-----------------------------------------------------------------------------
// -- begin detail:: --
//-----------------------------------------------------------------------------
namespace detail
{

template<typename T>
T* GetNodePointer(const conduit::Node& node);

template<>
float64* GetNodePointer<float64>(const conduit::Node& node)
{
  return const_cast<float64*>(node.as_float64_ptr());
}

template<>
float32* GetNodePointer<float32>(const conduit::Node& node)
{
  return const_cast<float32*>(node.as_float32_ptr());
}

template<>
int8* GetNodePointer<int8>(const conduit::Node& node)
{
  return const_cast<int8*>(node.as_int8_ptr());
}

template<>
int16* GetNodePointer<int16>(const conduit::Node& node)
{
  return const_cast<int16*>(node.as_int16_ptr());
}

template<>
int32* GetNodePointer<int32>(const conduit::Node& node)
{
  return const_cast<int32*>(node.as_int32_ptr());
}

template<>
int64* GetNodePointer<int64>(const conduit::Node& node)
{
  return const_cast<int64*>(node.as_int64_ptr());
}

template<>
uint8* GetNodePointer<uint8>(const conduit::Node& node)
{
  return const_cast<uint8*>(node.as_uint8_ptr());
}

template<>
uint16* GetNodePointer<uint16>(const conduit::Node& node)
{
  return const_cast<uint16*>(node.as_uint16_ptr());
}

template<>
uint32* GetNodePointer<uint32>(const conduit::Node& node)
{
  return const_cast<uint32*>(node.as_uint32_ptr());
}

template<>
uint64* GetNodePointer<uint64>(const conduit::Node& node)
{
  return const_cast<uint64*>(node.as_uint64_ptr());
}

template<typename T>
void CopyInterleavedArray(vtkDataArray*& arrayOut, const T* vals_ptr, const int size, int components, bool zero_copy)
{
  arrayOut = vtkDataArray::CreateDataArray(vtkTypeTraits<T>::VTKTypeID());
  arrayOut->SetNumberOfComponents(components);
  if (zero_copy)
  {
    arrayOut->SetVoidArray(const_cast<T*>(vals_ptr), static_cast<vtkIdType>(size), /*save*/ 1);
  }
  else
  {
    arrayOut->SetNumberOfTuples(size);
    memcpy(arrayOut->WriteVoidPointer(0, size), vals_ptr, size * sizeof(T));
  }
}

template<typename T>
void CopyNonInterleavedArray(
  vtkDataArray*& array_out, std::vector<T*> vals_ptrs, const int size, bool zero_copy)
{
  auto array_soa = vtkSOADataArrayTemplate<T>::New();
  array_out = array_soa;
  array_soa->SetNumberOfComponents(vals_ptrs.size());
  array_soa->SetNumberOfTuples(size);
  int ii = 0;
  for (auto val_ptr : vals_ptrs)
  {
    array_soa->SetArray(ii, val_ptr, size,
      /* updateMaxId */ false, /* save */ true,
      vtkAOSDataArrayTemplate<T>::VTK_DATA_ARRAY_FREE);
    ++ii;
  }
  if (!zero_copy)
  {
    vtkDataArray* tmp = array_out;
    array_out = vtkDataArray::CreateDataArray(vtkTypeTraits<T>::VTKTypeID());
    array_out->DeepCopy(array_out);
    tmp->Delete();
  }
}

template<typename N>
bool CopyInterleavedArray(vtkDataArray*& coords, const N& node, bool zero_copy)
{
  bool success = true;
  index_t atype = node.dtype().id();
  switch (atype)
  {
    case conduit::DataType::INT8_ID:
      CopyInterleavedArray(
        coords, node.as_int8_ptr(), node.dtype().number_of_elements(), /* num_components */1, zero_copy);
      break;
    case conduit::DataType::INT16_ID:
      CopyInterleavedArray(
        coords, node.as_int16_ptr(), node.dtype().number_of_elements(), /* num_components */1, zero_copy);
      break;
    case conduit::DataType::INT32_ID:
      CopyInterleavedArray(
        coords, node.as_int32_ptr(), node.dtype().number_of_elements(), /* num_components */1, zero_copy);
      break;
    case conduit::DataType::INT64_ID:
      CopyInterleavedArray(
        coords, node.as_int64_ptr(), node.dtype().number_of_elements(), /* num_components */1, zero_copy);
      break;
    case conduit::DataType::UINT8_ID:
      CopyInterleavedArray(
        coords, node.as_uint8_ptr(), node.dtype().number_of_elements(), /* num_components */1, zero_copy);
      break;
    case conduit::DataType::UINT16_ID:
      CopyInterleavedArray(
        coords, node.as_uint16_ptr(), node.dtype().number_of_elements(), /* num_components */1, zero_copy);
      break;
    case conduit::DataType::UINT32_ID:
      CopyInterleavedArray(
        coords, node.as_uint32_ptr(), node.dtype().number_of_elements(), /* num_components */1, zero_copy);
      break;
    case conduit::DataType::UINT64_ID:
      CopyInterleavedArray(
        coords, node.as_uint64_ptr(), node.dtype().number_of_elements(), /* num_components */1, zero_copy);
      break;
    case conduit::DataType::FLOAT32_ID:
      CopyInterleavedArray(
        coords, node.as_float32_ptr(), node.dtype().number_of_elements(), /* num_components */1, zero_copy);
      break;
    case conduit::DataType::FLOAT64_ID:
      CopyInterleavedArray(
        coords, node.as_float64_ptr(), node.dtype().number_of_elements(), /* num_components */1, zero_copy);
      break;
    case conduit::DataType::CHAR8_STR_ID:
      ASCENT_ERROR("Strings are unsupported.");
      success = false;
      break;

    case conduit::DataType::EMPTY_ID:
    case conduit::DataType::OBJECT_ID:
    case conduit::DataType::LIST_ID:
    default:
      ASCENT_ERROR("Empty and composite arrays are unsupported.");
      success = false;
      break;
  }

  return success;
}

template<typename T>
vtkPoints*
GetExplicitCoordinateSystem(
  const conduit::Node& n_coords,
  const std::string name,
  int& ndims,
  bool zero_copy)
{
  auto points_out = vtkPoints::New();
  vtkDataArray* coords_out = nullptr;

  int nverts = n_coords["values/x"].dtype().number_of_elements();
  bool is_interleaved = blueprint::mcarray::is_interleaved(n_coords["values"]);

  ndims = 2;

  T* x_coords_ptr = GetNodePointer<T>(n_coords["values/x"]);
  T* y_coords_ptr = GetNodePointer<T>(n_coords["values/y"]);
  T* z_coords_ptr = nullptr;

  if (n_coords.has_path("values/z"))
  {
    ndims = 3;
    z_coords_ptr = GetNodePointer<T>(n_coords["values/z"]);
  }

  if (!is_interleaved)
  {
    std::vector<T*> ptrs{x_coords_ptr, y_coords_ptr};

    if (ndims == 3)
    {
      ptrs.push_back(z_coords_ptr);
      CopyNonInterleavedArray(coords_out, ptrs, nverts, zero_copy);
    }
    else
    {
      vtkDataArray* tmp;
      CopyNonInterleavedArray(tmp, ptrs, nverts, /*zero_copy*/ true);
      coords_out = vtkDataArray::CreateDataArray(vtkTypeTraits<T>::VTKTypeID());
      coords_out->SetNumberOfComponents(3);
      coords_out->SetNumberOfTuples(tmp->GetNumberOfTuples());
      coords_out->CopyComponent(0, tmp, 0);
      coords_out->CopyComponent(1, tmp, 1);
      coords_out->FillComponent(2, 0.0);
      tmp->Delete();
    }
  }
  else
  {
    // we have interleaved coordinates x0,y0,z0,x1,y1,z1...
    const T* coords_ptr = GetNodePointer<T>(n_coords["values/x"]);
    // we cannot zero copy 2D interleaved arrays into vtkm
    // TODO: need way to detect 3d interleaved compendents that has
    //       only has xy in conduit
    if (ndims == 3 || true)
    {
      detail::CopyInterleavedArray(coords_out, coords_ptr, nverts, ndims, zero_copy);
    }
    else
    {
      vtkDataArray* two_dim;
      detail::CopyInterleavedArray(two_dim, coords_ptr, nverts, ndims, zero_copy);
      coords_out = vtkDataArray::CreateDataArray(two_dim->GetArrayType());
      coords_out->SetNumberOfComponents(ndims);
      coords_out->SetNumberOfTuples(nverts);
      coords_out->CopyComponent(0, two_dim, 0);
      coords_out->CopyComponent(1, two_dim, 1);
      coords_out->FillComponent(2, 0.0);

    }
  }
  points_out->SetData(coords_out);
  return points_out;
}

template<typename T>
bool GetField(
  vtkDataArray*& array_out,
  int& assoc_out,
  const conduit::Node& node,
  const std::string field_name,
  const std::string assoc_str,
  const std::string topo_str,
  bool zero_copy)
{
  if (assoc_str == "vertex")
  {
    assoc_out = vtkDataObject::FIELD_ASSOCIATION_POINTS;
  }
  else if (assoc_str == "element")
  {
    assoc_out = vtkDataObject::FIELD_ASSOCIATION_CELLS;
  }
  else
  {
    ASCENT_ERROR("Cannot add field association " << assoc_str);
  }

  int num_vals = node.dtype().number_of_elements();
  const T* values_ptr = node.value();

  array_out = nullptr;
  CopyInterleavedArray(array_out, values_ptr, 1, num_vals, zero_copy);
  return true;
}

int BlueprintToVTKCellType(const std::string& shape_name, int& indices, int& param_dimensions)
{
  int shape = VTK_EMPTY_CELL;
  if (shape_name == "point")
  {
    shape = VTK_VERTEX;
    indices = 1;
    param_dimensions = 0;
  }
  else if (shape_name == "line")
  {
    shape = VTK_LINE;
    indices = 2;
    param_dimensions = 1;
  }
  else if (shape_name == "tri")
  {
    shape = VTK_TRIANGLE;
    indices = 3;
    param_dimensions = 2;
  }
  else if (shape_name == "quad")
  {
    shape = VTK_QUAD;
    indices = 4;
    param_dimensions = 2;
  }
  else if (shape_name == "tet")
  {
    shape = VTK_TETRA;
    indices = 4;
    param_dimensions = 3;
  }
  else if (shape_name == "hex")
  {
    shape = VTK_HEXAHEDRON;
    indices = 8;
    param_dimensions = 3;
  }
  else if (shape_name == "polygonal")
  {
    shape = VTK_POLYGON;
    indices = -1; // no fixed number per cell
    param_dimensions = 2;
  }
  else if (shape_name == "polyhedral")
  {
    shape = VTK_POLYHEDRON;
    indices = -1; // no fixed number per cell
    param_dimensions = 3;
  }
  // TODO: Not supported in blueprint yet ...
  // else if (shape_name == "wedge")
  // {
  //   shape = VTK_WEDGE;
  //   indices = 6;
  //   param_dimensions = 3;
  // }
  // else if (shape_name == "pyramid")
  // {
  //   shape = VTK_PYRAMID;
  //   indices = 5;
  //   param_dimensions = 3;
  // }
  else
  {
    indices = -1;
    param_dimensions = -1;
    ASCENT_ERROR("Unsupported element shape " << shape_name);
  }
  return shape;
}

template<typename T>
void CopyConnectivity(vtkIdType*& dest, const T* ele_idx_ptr, int conn_size, int conn_per_cell)
{
  if (conn_per_cell > 0)
  {
    for (int ii = 0; ii < conn_size / conn_per_cell; ++ii)
    {
      *dest++ = conn_per_cell;
      for (int jj = 0; jj < conn_per_cell; ++jj)
      {
        *dest++ = static_cast<vtkIdType>(*ele_idx_ptr++);
      }
    }
  }
  else
  {
    for (int ii = 0; ii < conn_size; ++ii)
    {
      *dest++ = *ele_idx_ptr++;
    }
  }
}

template<typename T>
vtkIdType CellCountFromConnectivity(
  const T* ele_idx_ptr, vtkIdType conn_size, int vtk_cell_type, int conn_per_cell)
{
  vtkIdType num_cells;
  if (conn_per_cell > 0)
  {
    num_cells = conn_size / conn_per_cell;
  }
  else if (vtk_cell_type == VTK_POLYGON)
  {
    const T* end = ele_idx_ptr + conn_size;
    for (num_cells = 0; ele_idx_ptr < end; ++num_cells)
    {
      ele_idx_ptr += (*ele_idx_ptr + 1);
    }
  }
  else if (vtk_cell_type == VTK_POLYHEDRON)
  {
    const T* end = ele_idx_ptr + conn_size;
    for (num_cells = 0; ele_idx_ptr < end; ++num_cells)
    {
      int num_faces = *ele_idx_ptr++;
      for (int ff = 0; ff < num_faces && ele_idx_ptr < end; ++ff)
      {
        ele_idx_ptr += (*ele_idx_ptr + 1);
      }
    }
  }
  else
  {
    num_cells = 0;
    ASCENT_ERROR("Unsupported VTK cell type " << vtk_cell_type << " conn_per_cell " << conn_per_cell);
  }
  return num_cells;
}

}
//-----------------------------------------------------------------------------
// -- end detail:: --
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// VTKDataAdapter public methods
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
template<typename T>
void AddFieldDataValue(const std::string& fieldName, T fieldValue, vtkDataObject* dobj)
{
  if (fieldName.empty() || !dobj) { return; }

  auto field = vtkAbstractArray::CreateArray(vtkTypeTraits<T>::VTKTypeID());
  field->SetName(fieldName.c_str());
  field->SetNumberOfTuples(1);
  field->SetVariantValue(0, fieldValue);
  dobj->GetFieldData()->AddArray(field);
  field->FastDelete();
}

static void AddTimeStepInfo(const Node& dom, vtkDataObject* dobj)
{
  if (dom.has_path("state/time"))
  {
    double time = dom["state/time"].to_double();
    vtkDataObject::DATA_TIME_STEP()->Set(dobj->GetInformation(), time);
  }

  if (dom.has_path("state/cycle"))
  {
    vtkTypeUInt64 cycle = dom["state/cycle"].to_uint64();
    AddFieldDataValue("cycle", cycle, dobj);
  }
}

//-----------------------------------------------------------------------------
vtkMultiBlockDataSet*
VTKDataAdapter::BlueprintToVTKMultiBlock(
  const Node& node,
  bool zero_copy,
  const std::string& topo_name)
{
  // treat everything as a multi-block data set
  auto result = vtkMultiBlockDataSet::New();

  int num_domains = 0;

  // get the number of domains and check for id consistency
  num_domains = node.number_of_children();
  result->SetNumberOfBlocks(num_domains);
  AddTimeStepInfo(node, result);
  if (node.has_child("state/domain_id"))
  {
    int domain_id = node["state/domain_id"].to_int();
    AddFieldDataValue("domain_id", domain_id, result);
  }

  for (int i = 0; i < num_domains; ++i)
  {
    const conduit::Node& dom = node.child(i);
    vtkDataObject* dobj = VTKDataAdapter::BlueprintToVTKDataObject(
      dom, zero_copy, topo_name);

    int domain_id = dom["state/domain_id"].to_int();
    AddFieldDataValue("domain_id", domain_id, dobj);
    AddTimeStepInfo(dom, dobj);
    result->SetBlock(i, dobj);
  }
  return result;
}

//-----------------------------------------------------------------------------
vtkDataObject*
VTKDataAdapter::BlueprintToVTKDataObject(
  const Node& node,
  bool zero_copy,
  const std::string& topo_name_str)
{
  vtkDataObject* result = nullptr;

  std::string topo_name = topo_name_str;
  // if we don't specify a topology, find the first topology ...
  if (topo_name == "")
  {
    NodeConstIterator itr = node["topologies"].children();
    itr.next();
    topo_name = itr.name();
  }
  else
  {
    if (!node["topologies"].has_child(topo_name))
    {
      ASCENT_ERROR("Invalid topology name: " << topo_name);
    }
  }

  // As long as the mesh blueprint is verified, we can access
  // data without existence/error checks.
  const Node& n_topo   = node["topologies"][topo_name];
  string mesh_type     = n_topo["type"].as_string();

  string coords_name   = n_topo["coordset"].as_string();
  const Node& n_coords = node["coordsets"][coords_name];

  int neles  = 0;
  int nverts = 0;

  if (mesh_type == "uniform")
  {
    result = UniformBlueprintToVTKDataObject(coords_name,
      n_coords,
      topo_name,
      n_topo,
      neles,
      nverts);
  }
  else if (mesh_type == "rectilinear")
  {
    result = RectilinearBlueprintToVTKDataObject(coords_name,
      n_coords,
      topo_name,
      n_topo,
      neles,
      nverts,
      zero_copy);
  }
  else if (mesh_type == "structured")
  {
    result =  StructuredBlueprintToVTKDataObject(coords_name,
      n_coords,
      topo_name,
      n_topo,
      neles,
      nverts,
      zero_copy);
  }
  else if (mesh_type ==  "unstructured")
  {
    result =  UnstructuredBlueprintToVTKDataObject(coords_name,
      n_coords,
      topo_name,
      n_topo,
      neles,
      nverts,
      zero_copy);
  }
  else
  {
    ASCENT_ERROR("Unsupported topology/type:" << mesh_type);
    result = vtkDataObject::New();
  }

  // Add any fields:
  if (node.has_child("fields"))
  {
    NodeConstIterator itr = node["fields"].children();
    while (itr.has_next())
    {
      const Node& n_field = itr.next();
      std::string field_name = itr.name();

      // skip vector fields for now, we need to add
      // more logic to AddField
      const int num_children = n_field["values"].number_of_children();

      if (n_field["values"].number_of_children() == 0 )
      {
        AddField(field_name,
          n_field,
          topo_name,
          neles,
          nverts,
          result,
          zero_copy);
      }
    }
  }
  return result;
}


//-----------------------------------------------------------------------------
#if 0
class ExplicitArrayHelper
{
public:
// Helper function to create explicit connectivity arrays for vtkm data sets
void CreateExplicitArrays(vtkm::cont::ArrayHandle<vtkm::UInt8>& shapes,
                          vtkm::cont::ArrayHandle<vtkm::IdComponent>& num_indices,
                          const std::string& shape_type,
                          const vtkm::Id& conn_size,
                          vtkm::IdComponent& dimensionality,
                          int& neles)
{
    vtkm::UInt8 shape_id = 0;
    vtkm::IdComponent indices= 0;
    if (shape_type == "tri")
    {
        shape_id = 3;
        indices = 3;
        // note: vtkm cell dimensions are topological
        dimensionality = 2;
    }
    else if (shape_type == "quad")
    {
        shape_id = 9;
        indices = 4;
        // note: vtkm cell dimensions are topological
        dimensionality = 2;
    }
    else if (shape_type == "tet")
    {
        shape_id = 10;
        indices = 4;
        dimensionality = 3;
    }
    else if (shape_type == "hex")
    {
        shape_id = 12;
        indices = 8;
        dimensionality = 3;
    }
    // TODO: Not supported in blueprint yet ...
    // else if (shape_type == "wedge")
    // {
    //     shape_id = 13;
    //     indices = 6;
    //     dimensionality = 3;
    // }
    // else if (shape_type == "pyramid")
    // {
    //     shape_id = 14;
    //     indices = 5;
    //     dimensionality = 3;
    // }
    else
    {
        ASCENT_ERROR("Unsupported element shape " << shape_type);
    }

    if (conn_size < indices)
        ASCENT_ERROR("Connectivity array size " << conn_size << " must be at least size " << indices);
    if (conn_size % indices != 0)
        ASCENT_ERROR("Connectivity array size " << conn_size << " be evenly divided by indices size" << indices);

    const vtkm::Id num_shapes = conn_size / indices;

    neles = num_shapes;

    shapes.Allocate(num_shapes);
    num_indices.Allocate(num_shapes);

    // We could memset these and then zero copy them but that
    // would make us responsible for the data. If we just create
    // them, smart pointers will automatically delete them.
    // Hopefull the compiler turns this into a memset.

    const vtkm::UInt8 shape_value = shape_id;
    const vtkm::IdComponent indices_value = indices;
    auto shapes_portal = shapes.GetPortalControl();
    auto num_indices_portal = num_indices.GetPortalControl();
#ifdef ASCENT_USE_OPENMP
    #pragma omp parallel for
#endif
    for (int i = 0; i < num_shapes; ++i)
    {
        shapes_portal.Set(i, shape_value);
        num_indices_portal.Set(i, indices_value);
    }
}
};
#endif // 0
//-----------------------------------------------------------------------------

vtkDataObject*
VTKDataAdapter::UniformBlueprintToVTKDataObject(
  const std::string& coords_name, // input string with coordset name
  const Node& n_coords,           // input mesh bp coordset (assumed uniform)
  const std::string& topo_name,   // input string with topo name
  const Node& n_topo,             // input mesh bp topo
  int& neles,                     // output, number of eles
  int& nverts)                    // output, number of verts
{
  //
  // blueprint uniform coord set provides:
  //
  //  dims/{i,j,k}
  //  origin/{x,y,z} (optional)
  //  spacing/{dx,dy,dz} (optional)

  auto result = vtkImageData::New();

  const Node& n_dims = n_coords["dims"];

  int dims_i = n_dims["i"].to_int();
  int dims_j = n_dims["j"].to_int();
  int dims_k = 1;

  bool is_2d = true;

  // check for 3d
  if (n_dims.has_path("k"))
  {
    dims_k = n_dims["k"].to_int();
    is_2d = false;
  }

  float64 origin_x = 0.0;
  float64 origin_y = 0.0;
  float64 origin_z = 0.0;

  float64 spacing_x = 1.0;
  float64 spacing_y = 1.0;
  float64 spacing_z = 1.0;

  if (n_coords.has_child("origin"))
  {
    const Node& n_origin = n_coords["origin"];

    if (n_origin.has_child("x"))
    {
      origin_x = n_origin["x"].to_float64();
    }

    if (n_origin.has_child("y"))
    {
      origin_y = n_origin["y"].to_float64();
    }

    if (n_origin.has_child("z"))
    {
      origin_z = n_origin["z"].to_float64();
    }
  }

  if (n_coords.has_path("spacing"))
  {
    const Node& n_spacing = n_coords["spacing"];

    if (n_spacing.has_path("dx"))
    {
      spacing_x = n_spacing["dx"].to_float64();
    }

    if (n_spacing.has_path("dy"))
    {
      spacing_y = n_spacing["dy"].to_float64();
    }

    if (n_spacing.has_path("dz"))
    {
      spacing_z = n_spacing["dz"].to_float64();
    }
  }

  result->SetOrigin(origin_x, origin_y, origin_z);
  result->SetSpacing(spacing_x, spacing_y, spacing_z);
  result->SetDimensions(dims_i, dims_j, dims_k);

  neles =  (dims_i - 1) * (dims_j - 1);
  if (dims_k > 1)
  {
    neles *= (dims_k - 1);
  }

  nverts =  dims_i * dims_j;
  if (dims_k > 1)
  {
    nverts *= dims_k;
  }

  return result;
}

//-----------------------------------------------------------------------------

vtkRectilinearGrid*
VTKDataAdapter::RectilinearBlueprintToVTKDataObject(
  const std::string& coords_name, // input string with coordset name
  const Node& n_coords,           // input mesh bp coordset (assumed rectilinear)
  const std::string& topo_name,   // input string with topo name
  const Node& n_topo,             // input mesh bp topo
  int& neles,                     // output, number of eles
  int& nverts,                    // output, number of verts
  bool zero_copy)                 // attempt to zero copy
{
  auto result = vtkRectilinearGrid::New();

  int x_npts = n_coords["values/x"].dtype().number_of_elements();
  int y_npts = n_coords["values/y"].dtype().number_of_elements();
  int z_npts = 0;

  int32 ndims = 2;

  vtkDataArray* x_coords = nullptr;
  vtkDataArray* y_coords = nullptr;
  vtkDataArray* z_coords = nullptr;
  detail::CopyInterleavedArray(x_coords, n_coords["values/x"], zero_copy);
  detail::CopyInterleavedArray(y_coords, n_coords["values/y"], zero_copy);
  if (n_coords.has_path("values/z"))
  {
    ndims = 3;
    z_npts = n_coords["values/z"].dtype().number_of_elements();
    detail::CopyInterleavedArray(z_coords, n_coords["values/z"], zero_copy);
  }
  else
  {
    auto z_tmp = vtkFloatArray::New();
    z_tmp->SetNumberOfTuples(1);
    z_tmp->SetValue(0, 0.0);
    z_coords = z_tmp;
  }

  result->SetDimensions(
    x_coords->GetNumberOfTuples(),
    y_coords->GetNumberOfTuples(),
    z_coords->GetNumberOfTuples());

  result->SetXCoordinates(x_coords);
  result->SetYCoordinates(y_coords);
  result->SetZCoordinates(z_coords);

  nverts = x_npts * y_npts;
  neles = (x_npts - 1) * (y_npts - 1);
  if (ndims > 2)
  {
    nverts *= z_npts;
    neles *= (z_npts - 1);
  }

  return result;
}

//-----------------------------------------------------------------------------

vtkStructuredGrid*
VTKDataAdapter::StructuredBlueprintToVTKDataObject(
  const std::string& coords_name, // input string with coordset name
  const Node& n_coords,           // input mesh bp coordset (assumed rectilinear)
  const std::string& topo_name,   // input string with topo name
  const Node& n_topo,             // input mesh bp topo
  int& neles,                     // output, number of eles
  int& nverts,                    // output, number of verts
  bool zero_copy)                 // attempt to zero copy
{
  auto result = vtkStructuredGrid::New();

  nverts = n_coords["values/x"].dtype().number_of_elements();

  int ndims = 0;

  vtkPoints* coords = nullptr;
  if (n_coords["values/x"].dtype().is_float64())
  {
    coords = detail::GetExplicitCoordinateSystem<float64>(n_coords,
      coords_name,
      ndims,
      zero_copy);
  }
  else if (n_coords["values/x"].dtype().is_float32())
  {
    coords = detail::GetExplicitCoordinateSystem<float32>(n_coords,
      coords_name,
      ndims,
      zero_copy);
  }
  else
  {
    ASCENT_ERROR("Coordinate system must be floating point values");
  }

  result->SetPoints(coords);

  int32 x_elems = n_topo["elements/dims/i"].as_int32();
  int32 y_elems = n_topo["elements/dims/j"].as_int32();
  if (ndims == 2)
  {
    neles = x_elems * y_elems;
    result->SetDimensions(x_elems + 1, y_elems + 1, 1);
  }
  else
  {
    int32 z_elems = n_topo["elements/dims/k"].as_int32();
    neles = x_elems * y_elems * z_elems;
    result->SetDimensions(x_elems + 1, y_elems + 1, z_elems + 1);
  }
  return result;
}

//-----------------------------------------------------------------------------

vtkUnstructuredGrid*
VTKDataAdapter::UnstructuredBlueprintToVTKDataObject(
  const std::string& coords_name, // input string with coordset name
  const Node& n_coords,           // input mesh bp coordset (assumed unstructured)
  const std::string& topo_name,   // input string with topo name
  const Node& n_topo,             // input mesh bp topo
  int& neles,                     // output, number of eles
  int& nverts,                    // output, number of verts
  bool zero_copy)                 // attempt to zero copy
{
  auto result = vtkUnstructuredGrid::New();

  nverts = n_coords["values/x"].dtype().number_of_elements();

  int32 ndims;
  vtkPoints* coords = nullptr;
  if (n_coords["values/x"].dtype().is_float64())
  {
    coords = detail::GetExplicitCoordinateSystem<float64>(n_coords,
      coords_name,
      ndims,
      zero_copy);
  }
  else if (n_coords["values/x"].dtype().is_float32())
  {
    coords = detail::GetExplicitCoordinateSystem<float32>(n_coords,
      coords_name,
      ndims,
      zero_copy);
  }
  else
  {
    ASCENT_ERROR("Coordinate system must be floating point values.");
  }
  result->SetPoints(coords);

  // II. Connectivity and cell type arrays.
  //
  // Note that vtkUnstructuredGrid uses vtkCellArray to hold
  // point IDs which also interleaves the number of points per
  // cell, making it impossible to zero-copy the connectivity
  // as provided by conduit/blueprint.
  const Node& n_topo_eles = n_topo["elements"];
  std::string ele_shape = n_topo_eles["shape"].as_string();
  const Node& n_topo_conn = n_topo_eles["connectivity"];
  vtkIdType conn_size = static_cast<vtkIdType>(n_topo_conn.dtype().number_of_elements());
  int conn_per_cell;
  int vtk_cell_type;
  int dim;
  if ((vtk_cell_type = detail::BlueprintToVTKCellType(ele_shape, conn_per_cell, dim)) == VTK_EMPTY_CELL)
  {
    return result;
  }

  auto conn = vtkSmartPointer<vtkCellArray>::New();
  if (n_topo_conn.dtype().is_int32() && n_topo_conn.is_compact())
  {
    const auto ele_idx_ptr = static_cast<const int32_t*>(n_topo_conn.data_ptr());
    vtkIdType cell_size = detail::CellCountFromConnectivity(ele_idx_ptr, conn_size, vtk_cell_type, conn_per_cell);
    auto cptr = conn->WritePointer(cell_size, conn_size + cell_size);
    detail::CopyConnectivity(cptr, ele_idx_ptr, conn_size, conn_per_cell);
    neles = static_cast<int>(cell_size);
  }
  else if (n_topo_conn.dtype().is_int64() && n_topo_conn.is_compact())
  {
    const auto ele_idx_ptr = static_cast<const int64_t*>(n_topo_conn.data_ptr());
    vtkIdType cell_size = detail::CellCountFromConnectivity(ele_idx_ptr, conn_size, vtk_cell_type, conn_per_cell);
    auto cptr = conn->WritePointer(cell_size, conn_size + cell_size);
    detail::CopyConnectivity(cptr, ele_idx_ptr, conn_size, conn_per_cell);
    neles = static_cast<int>(cell_size);
  }
  else
  {
    ASCENT_ERROR("Unsupported connectivity size/storage.");
    return result;
  }

  result->SetCells(vtk_cell_type, conn);
  ASCENT_INFO("neles "  << neles);

  return result;
}

//-----------------------------------------------------------------------------

void
VTKDataAdapter::AddField(
  const std::string& field_name,
  const Node& n_field,
  const std::string& topo_name,
  int neles,
  int nverts,
  vtkDataObject* dobj,
  bool zero_copy) // attempt to zero copy
{
  // TODO: how do we deal with vector valued fields?, these will be mcarrays

  vtkDataSet* dset = vtkDataSet::SafeDownCast(dobj);
  vtkDataArray* array;
  string assoc_str = n_field["association"].as_string();
  const Node& n_vals = n_field["values"];
  detail::CopyInterleavedArray(array, n_vals, zero_copy);
  array->SetName(field_name.c_str());

  int assoc = vtkDataObject::FIELD_ASSOCIATION_NONE;
  if (assoc_str == "vertex")
  {
    assoc = vtkDataObject::FIELD_ASSOCIATION_POINTS;
    if (dset && array)
    {
      dset->GetPointData()->AddArray(array);
    }
  }
  else if (assoc_str == "element")
  {
    assoc = vtkDataObject::FIELD_ASSOCIATION_CELLS;
    if (dset && array)
    {
      dset->GetCellData()->AddArray(array);
    }
  }
  else
  {
    ASCENT_INFO("VTK conversion does not support field assoc " << assoc_str << ". Skipping");
    std::cout<<"VTKm conversion does not support field assoc " << assoc_str << ". Skipping\n";
    return;
  }

  int num_vals = n_vals.dtype().number_of_elements();

  // if assoc == "vertex"   check that num_vals == nverts;
  // if assoc == "element"  check that num_vals == neles;

  ASCENT_INFO("field association: "      << assoc_str);
  ASCENT_INFO("number of field values: " << num_vals);
  ASCENT_INFO("number of vertices: "     << nverts);
  ASCENT_INFO("number of elements: "     << neles);
}

#if 0
std::string
GetBlueprintCellName(vtkm::UInt8 shape_id)
{
  std::string name;
  if (shape_id == vtkm::CELL_SHAPE_TRIANGLE)
  {
    name = "tri";
  }
  else if (shape_id == vtkm::CELL_SHAPE_VERTEX)
  {
    name = "point";
  }
  else if (shape_id == vtkm::CELL_SHAPE_LINE)
  {
    name = "line";
  }
  else if (shape_id == vtkm::CELL_SHAPE_POLYGON)
  {
    ASCENT_ERROR("Polygon is not supported in blueprint");
  }
  else if (shape_id == vtkm::CELL_SHAPE_QUAD)
  {
    name = "quad";
  }
  else if (shape_id == vtkm::CELL_SHAPE_TETRA)
  {
    name = "tet";
  }
  else if (shape_id == vtkm::CELL_SHAPE_HEXAHEDRON)
  {
    name = "hex";
  }
  else if (shape_id == vtkm::CELL_SHAPE_WEDGE)
  {
    ASCENT_ERROR("Wedge is not supported in blueprint");
  }
  else if (shape_id == vtkm::CELL_SHAPE_PYRAMID)
  {
    ASCENT_ERROR("Pyramid is not supported in blueprint");
  }
  return name;
}

bool
VTKDataAdapter::VTKmTopologyToBlueprint(conduit::Node& output,
                                         const vtkm::cont::DataSet& data_set)
{

  const int default_cell_set = 0;
  int topo_dims;
  bool is_structured = vtkh::VTKMDataSetInfo::IsStructured(data_set, topo_dims, default_cell_set);
  bool is_uniform = vtkh::VTKMDataSetInfo::IsUniform(data_set);
  bool is_rectilinear = vtkh::VTKMDataSetInfo::IsRectilinear(data_set);
  vtkm::cont::CoordinateSystem coords = data_set.GetCoordinateSystem();
  // we cannot access an empty domain
  bool is_empty = false;

  if (data_set.GetCoordinateSystem().GetData().GetNumberOfValues() == 0 ||
     data_set.GetCellSet().GetNumberOfCells() == 0)
  {
    is_empty = true;
  }

  if (is_empty)
  {
    return is_empty;
  }

  if (is_uniform)
  {
    auto points = coords.GetData().Cast<vtkm::cont::ArrayHandleUniformPointCoordinates>();
    auto portal = points.GetPortalConstControl();

    auto origin = portal.GetOrigin();
    auto spacing = portal.GetSpacing();
    auto dims = portal.GetDimensions();
    output["topologies/topo/coordset"] = "coords";
    output["topologies/topo/type"] = "uniform";

    output["coordsets/coords/type"] = "uniform";
    output["coordsets/coords/dims/i"] = (int) dims[0];
    output["coordsets/coords/dims/j"] = (int) dims[1];
    output["coordsets/coords/dims/k"] = (int) dims[2];
    output["coordsets/coords/origin/x"] = (int) origin[0];
    output["coordsets/coords/origin/y"] = (int) origin[1];
    output["coordsets/coords/origin/z"] = (int) origin[2];
    output["coordsets/coords/spacing/x"] = (int) spacing[0];
    output["coordsets/coords/spacing/y"] = (int) spacing[1];
    output["coordsets/coords/spacing/z"] = (int) spacing[2];
  }
  else if (is_rectilinear)
  {
    typedef vtkm::cont::ArrayHandleCartesianProduct<vtkm::cont::ArrayHandle<vtkm::FloatDefault>,
                                                    vtkm::cont::ArrayHandle<vtkm::FloatDefault>,
                                                    vtkm::cont::ArrayHandle<vtkm::FloatDefault>> Cartesian;

    typedef vtkm::cont::ArrayHandle<vtkm::FloatDefault> HandleType;
    typedef typename HandleType::template ExecutionTypes<vtkm::cont::DeviceAdapterTagSerial>::PortalConst PortalType;
    typedef typename vtkm::cont::ArrayPortalToIterators<PortalType>::IteratorType IteratorType;

    const auto points = coords.GetData().Cast<Cartesian>();
    auto portal = points.GetPortalConstControl();
    auto x_portal = portal.GetFirstPortal();
    auto y_portal = portal.GetSecondPortal();
    auto z_portal = portal.GetThirdPortal();

    IteratorType x_iter = vtkm::cont::ArrayPortalToIterators<PortalType>(x_portal).GetBegin();
    IteratorType y_iter = vtkm::cont::ArrayPortalToIterators<PortalType>(y_portal).GetBegin();
    IteratorType z_iter = vtkm::cont::ArrayPortalToIterators<PortalType>(z_portal).GetBegin();
    // work around for conduit not accepting const pointers
    vtkm::FloatDefault* x_ptr = const_cast<vtkm::FloatDefault*>(&(*x_iter));
    vtkm::FloatDefault* y_ptr = const_cast<vtkm::FloatDefault*>(&(*y_iter));
    vtkm::FloatDefault* z_ptr = const_cast<vtkm::FloatDefault*>(&(*z_iter));

    output["topologies/topo/coordset"] = "coords";
    output["topologies/topo/type"] = "rectilinear";

    output["coordsets/coords/type"] = "rectilinear";
    output["coordsets/coords/values/x"].set(x_ptr, x_portal.GetNumberOfValues());
    output["coordsets/coords/values/y"].set(y_ptr, y_portal.GetNumberOfValues());
    output["coordsets/coords/values/z"].set(z_ptr, z_portal.GetNumberOfValues());
  }
  else
  {
    int point_dims[3];
    //
    // This still could be structured, but this will always
    // have an explicit coordinate system
    output["coordsets/coords/type"] = "explicit";
    using Coords32 = vtkm::cont::ArrayHandleCompositeVector<vtkm::cont::ArrayHandle<vtkm::Float32>,
                                                            vtkm::cont::ArrayHandle<vtkm::Float32>,
                                                            vtkm::cont::ArrayHandle<vtkm::Float32>>;

    using Coords64 = vtkm::cont::ArrayHandleCompositeVector<vtkm::cont::ArrayHandle<vtkm::Float64>,
                                                            vtkm::cont::ArrayHandle<vtkm::Float64>,
                                                            vtkm::cont::ArrayHandle<vtkm::Float64>>;

    using CoordsVec32 = vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Float32,3>>;
    using CoordsVec64 = vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Float64,3>>;

    bool coords_32 = coords.GetData().IsSameType(Coords32());


    if (coords.GetData().IsSameType(Coords32()))
    {
      Coords32 points = coords.GetData().Cast<Coords32>();

      auto x_handle = vtkmstd::get<0>(points.GetStorage().GetArrayTuple());
      auto y_handle = vtkmstd::get<1>(points.GetStorage().GetArrayTuple());
      auto z_handle = vtkmstd::get<2>(points.GetStorage().GetArrayTuple());

      point_dims[0] = x_handle.GetNumberOfValues();
      point_dims[1] = y_handle.GetNumberOfValues();
      point_dims[2] = z_handle.GetNumberOfValues();
      output["coordsets/coords/values/x"].set(vtkh::GetVTKMPointer(x_handle), point_dims[0]);
      output["coordsets/coords/values/y"].set(vtkh::GetVTKMPointer(y_handle), point_dims[1]);
      output["coordsets/coords/values/z"].set(vtkh::GetVTKMPointer(z_handle), point_dims[2]);

    }
    else if (coords.GetData().IsSameType(CoordsVec32()))
    {
      CoordsVec32 points = coords.GetData().Cast<CoordsVec32>();

      const int num_vals = points.GetNumberOfValues();
      vtkm::Float32* points_ptr = (vtkm::Float32*)vtkh::GetVTKMPointer(points);
      const int byte_size = sizeof(vtkm::Float32);

      output["coordsets/coords/values/x"].set(points_ptr,
                                              num_vals,
                                              byte_size*0,  // byte offset
                                              byte_size*3); // stride
      output["coordsets/coords/values/y"].set(points_ptr,
                                              num_vals,
                                              byte_size*1,  // byte offset
                                              sizeof(vtkm::Float32)*3); // stride
      output["coordsets/coords/values/z"].set(points_ptr,
                                              num_vals,
                                              byte_size*2,  // byte offset
                                              byte_size*3); // stride

    }
    else if (coords.GetData().IsSameType(Coords64()))
    {
      Coords64 points = coords.GetData().Cast<Coords64>();

      auto x_handle = vtkmstd::get<0>(points.GetStorage().GetArrayTuple());
      auto y_handle = vtkmstd::get<1>(points.GetStorage().GetArrayTuple());
      auto z_handle = vtkmstd::get<2>(points.GetStorage().GetArrayTuple());

      point_dims[0] = x_handle.GetNumberOfValues();
      point_dims[1] = y_handle.GetNumberOfValues();
      point_dims[2] = z_handle.GetNumberOfValues();
      output["coordsets/coords/values/x"].set(vtkh::GetVTKMPointer(x_handle), point_dims[0]);
      output["coordsets/coords/values/y"].set(vtkh::GetVTKMPointer(y_handle), point_dims[1]);
      output["coordsets/coords/values/z"].set(vtkh::GetVTKMPointer(z_handle), point_dims[2]);

    }
    else if (coords.GetData().IsSameType(CoordsVec64()))
    {
      CoordsVec64 points = coords.GetData().Cast<CoordsVec64>();

      const int num_vals = points.GetNumberOfValues();
      vtkm::Float64* points_ptr = (vtkm::Float64*)vtkh::GetVTKMPointer(points);
      const int byte_size = sizeof(vtkm::Float64);

      output["coordsets/coords/values/x"].set(points_ptr,
                                              num_vals,
                                              byte_size*0,  // byte offset
                                              byte_size*3); // stride
      output["coordsets/coords/values/y"].set(points_ptr,
                                              num_vals,
                                              byte_size*1,  // byte offset
                                              byte_size*3); // stride
      output["coordsets/coords/values/z"].set(points_ptr,
                                              num_vals,
                                              byte_size*2,  // byte offset
                                              byte_size*3); // stride

    }
    else
    {
      coords.PrintSummary(std::cerr);
      ASCENT_ERROR("Unknown coords type");
    }

    if (is_structured)
    {
      output["topologies/topo/coordset"] = "coords";
      output["topologies/topo/type"] = "structured";
      output["topologies/topo/elements/dims/i"] = (int) point_dims[0];
      output["topologies/topo/elements/dims/j"] = (int) point_dims[1];
      output["topologies/topo/elements/dims/k"] = (int) point_dims[2];
    }
    else
    {
      output["topologies/topo/coordset"] = "coords";
      output["topologies/topo/type"] = "unstructured";
      vtkm::cont::DynamicCellSet dyn_cells = data_set.GetCellSet();

      using SingleType = vtkm::cont::CellSetSingleType<>;
      using MixedType = vtkm::cont::CellSetExplicit<>;

      if (dyn_cells.IsSameType(SingleType()))
      {
        SingleType cells = dyn_cells.Cast<SingleType>();
        vtkm::UInt8 shape_id = cells.GetCellShape(0);
        std::string conduit_name = GetBlueprintCellName(shape_id);
        output["topologies/topo/elements/shape"] = conduit_name;

        static_assert(sizeof(vtkm::Id) == sizeof(int), "blueprint expects connectivity to be ints");
        auto conn = cells.GetConnectivityArray(vtkm::TopologyElementTagPoint(),
                                               vtkm::TopologyElementTagCell());

        output["topologies/topo/elements/connectivity"].set(vtkh::GetVTKMPointer(conn),
                                                             conn.GetNumberOfValues());
      }
      else if (vtkh::VTKMDataSetInfo::IsSingleCellShape(dyn_cells))
      {
        // If we are here, the we know that the cell set is explicit,
        // but only a single cell shape
        auto cells = dyn_cells.Cast<vtkm::cont::CellSetExplicit<>>();
        auto shapes = cells.GetShapesArray(vtkm::TopologyElementTagPoint(),
                                           vtkm::TopologyElementTagCell());

        vtkm::UInt8 shape_id = shapes.GetPortalControl().Get(0);

        std::string conduit_name = GetBlueprintCellName(shape_id);
        output["topologies/topo/elements/shape"] = conduit_name;

        static_assert(sizeof(vtkm::Id) == sizeof(int), "blueprint expects connectivity to be ints");

        auto conn = cells.GetConnectivityArray(vtkm::TopologyElementTagPoint(),
                                               vtkm::TopologyElementTagCell());

        output["topologies/topo/elements/connectivity"].set(vtkh::GetVTKMPointer(conn),
                                                             conn.GetNumberOfValues());

      }
      else
      {
        ASCENT_ERROR("Mixed explicit types not implemented");
        MixedType cells = dyn_cells.Cast<MixedType>();
      }

    }
  }
  return is_empty;
}

template<typename T, int N>
void ConvertVecToNode(conduit::Node& output,
                      std::string path,
                      vtkm::cont::ArrayHandle<vtkm::Vec<T,N>>& handle)
{
  static_assert(N > 1 && N < 4, "Vecs must be size 2 or 3");
  output[path + "/type"] = "vector";
  output[path + "/values/u"].set((T*) vtkh::GetVTKMPointer(handle),
                                 handle.GetNumberOfValues(),
                                 sizeof(T)*0,   // starting offset in bytes
                                 sizeof(T)*N);  // stride in bytes
  output[path + "/values/v"].set((T*) vtkh::GetVTKMPointer(handle),
                                 handle.GetNumberOfValues(),
                                 sizeof(T)*1,   // starting offset in bytes
                                 sizeof(T)*N);  // stride in bytes
  if (N == 3)
  {

    output[path + "/values/w"].set((T*) vtkh::GetVTKMPointer(handle),
                                   handle.GetNumberOfValues(),
                                   sizeof(T)*2,   // starting offset in bytes
                                   sizeof(T)*N);  // stride in bytes
  }
}

void
VTKDataAdapter::VTKmFieldToBlueprint(conduit::Node& output,
                                      const vtkm::cont::Field& field)
{
  std::string name = field.GetName();
  std::string path = "fields/" + name;
  bool assoc_points = vtkm::cont::Field::Association::POINTS == field.GetAssociation();
  bool assoc_cells  = vtkm::cont::Field::Association::CELL_SET == field.GetAssociation();
  //bool assoc_mesh  = vtkm::cont::Field::ASSOC_WHOLE_MESH == field.GetAssociation();
  if (!assoc_points && ! assoc_cells)
  {
    ASCENT_ERROR("Field must be associtated with cells or points\n");
  }
  std::string conduit_name;

  if (assoc_points) conduit_name = "vertex";
  else conduit_name = "element";

  output[path + "/association"] = conduit_name;
  output[path + "/topology"] = "topo";

  vtkm::cont::DynamicArrayHandle dyn_handle = field.GetData();
  //
  // this can be literally anything. Lets do some exhaustive casting
  //
  if (dyn_handle.IsSameType(vtkm::cont::ArrayHandle<vtkm::Float32>()))
  {
    using HandleType = vtkm::cont::ArrayHandle<vtkm::Float32>;
    HandleType handle = dyn_handle.Cast<HandleType>();
    output[path + "/values"].set(vtkh::GetVTKMPointer(handle), handle.GetNumberOfValues());
  }
  else if (dyn_handle.IsSameType(vtkm::cont::ArrayHandle<vtkm::Float64>()))
  {
    using HandleType = vtkm::cont::ArrayHandle<vtkm::Float64>;
    HandleType handle = dyn_handle.Cast<HandleType>();
    output[path + "/values"].set(vtkh::GetVTKMPointer(handle), handle.GetNumberOfValues());
  }
  else if (dyn_handle.IsSameType(vtkm::cont::ArrayHandle<vtkm::Int8>()))
  {
    using HandleType = vtkm::cont::ArrayHandle<vtkm::Int8>;
    HandleType handle = dyn_handle.Cast<HandleType>();
    output[path + "/values"].set(vtkh::GetVTKMPointer(handle), handle.GetNumberOfValues());
  }
  else if (dyn_handle.IsSameType(vtkm::cont::ArrayHandle<vtkm::Int32>()))
  {
    using HandleType = vtkm::cont::ArrayHandle<vtkm::Int32>;
    HandleType handle = dyn_handle.Cast<HandleType>();
    output[path + "/values"].set(vtkh::GetVTKMPointer(handle), handle.GetNumberOfValues());
  }
  else if (dyn_handle.IsSameType(vtkm::cont::ArrayHandle<vtkm::Int64>()))
  {
    using HandleType = vtkm::cont::ArrayHandle<vtkm::Int64>;
    HandleType handle = dyn_handle.Cast<HandleType>();
    ASCENT_ERROR("Conduit int64 and vtkm::Int64 are different. Cannot convert vtkm::Int64\n");
    //output[path + "/values"].set(vtkh::GetVTKMPointer(handle), handle.GetNumberOfValues());
  }
  else if (dyn_handle.IsSameType(vtkm::cont::ArrayHandle<vtkm::UInt32>()))
  {
    using HandleType = vtkm::cont::ArrayHandle<vtkm::UInt32>;
    HandleType handle = dyn_handle.Cast<HandleType>();
    output[path + "/values"].set(vtkh::GetVTKMPointer(handle), handle.GetNumberOfValues());
  }
  else if (dyn_handle.IsSameType(vtkm::cont::ArrayHandle<vtkm::UInt8>()))
  {
    using HandleType = vtkm::cont::ArrayHandle<vtkm::UInt8>;
    HandleType handle = dyn_handle.Cast<HandleType>();
    output[path + "/values"].set(vtkh::GetVTKMPointer(handle), handle.GetNumberOfValues());
  }
  else if (dyn_handle.IsSameType(vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Float32,3>>()))
  {
    using HandleType = vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Float32,3>>;
    HandleType handle = dyn_handle.Cast<HandleType>();
    ConvertVecToNode(output, path, handle);
  }
  else if (dyn_handle.IsSameType(vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Float64,3>>()))
  {
    using HandleType = vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Float64,3>>;
    HandleType handle = dyn_handle.Cast<HandleType>();
    ConvertVecToNode(output, path, handle);
  }
  else if (dyn_handle.IsSameType(vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Int32,3>>()))
  {
    using HandleType = vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Int32,3>>;
    HandleType handle = dyn_handle.Cast<HandleType>();
    ConvertVecToNode(output, path, handle);
  }
  else if (dyn_handle.IsSameType(vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Float32,2>>()))
  {
    using HandleType = vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Float32,2>>;
    HandleType handle = dyn_handle.Cast<HandleType>();
    ConvertVecToNode(output, path, handle);
  }
  else if (dyn_handle.IsSameType(vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Float64,2>>()))
  {
    using HandleType = vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Float64,2>>;
    HandleType handle = dyn_handle.Cast<HandleType>();
    ConvertVecToNode(output, path, handle);
  }
  else if (dyn_handle.IsSameType(vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Int32,2>>()))
  {
    using HandleType = vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Int32,2>>;
    HandleType handle = dyn_handle.Cast<HandleType>();
    ConvertVecToNode(output, path, handle);
  }
  else
  {
    field.PrintSummary(std::cerr);
    ASCENT_ERROR("Field type unsupported for conversion");
  }
}

void
VTKDataAdapter::VTKmToBlueprintDataSet(const vtkm::cont::DataSet* dset,
                                        conduit::Node& node)
{
  //
  // with vtkm, we have no idea what the type is of anything inside
  // dataset, so we have to ask all fields, cell sets anc coordinate systems.
  //
  const int default_cell_set = 0;

  bool is_empty = VTKmTopologyToBlueprint(node, *dset);

  if (!is_empty)
  {
    const vtkm::Id num_fields = dset->GetNumberOfFields();
    for(vtkm::Id i = 0; i < num_fields; ++i)
    {
      vtkm::cont::Field field = dset->GetField(i);
      VTKmFieldToBlueprint(node, field);
    }
  }
}
#endif // 0

}
//-----------------------------------------------------------------------------
// -- end ascent:: --
//-----------------------------------------------------------------------------
