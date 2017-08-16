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
/// file: alpine_runtime_vtkh_filters.cpp
///
//-----------------------------------------------------------------------------

#include "alpine_runtime_vtkh_filters.hpp"

//-----------------------------------------------------------------------------
// thirdparty includes
//-----------------------------------------------------------------------------

// conduit includes
#include <conduit.hpp>
#include <conduit_relay.hpp>
#include <conduit_blueprint.hpp>

//-----------------------------------------------------------------------------
// alpine includes
//-----------------------------------------------------------------------------
#include <alpine_logging.hpp>
#include <flow_graph.hpp>
#include <flow_workspace.hpp>

#if defined(ALPINE_VTKM_ENABLED)
#include <vtkh.hpp>
#include <vtkh_data_set.hpp>
#include <rendering/vtkh_renderer_ray_tracer.hpp>
#include <rendering/vtkh_renderer_volume.hpp>
#include <vtkh_clip.hpp>
#include <vtkh_marching_cubes.hpp>
#include <vtkh_threshold.hpp>
#include <vtkm/cont/DataSet.h>

#include <alpine_data_adapter.hpp>

#endif

using namespace conduit;
using namespace std;

using namespace flow;

//-----------------------------------------------------------------------------
// -- begin alpine:: --
//-----------------------------------------------------------------------------
namespace alpine
{

//-----------------------------------------------------------------------------
// -- begin alpine::runtime --
//-----------------------------------------------------------------------------
namespace runtime
{

//-----------------------------------------------------------------------------
// -- begin alpine::runtime::filters --
//-----------------------------------------------------------------------------
namespace filters
{


//-----------------------------------------------------------------------------
EnsureVTKH::EnsureVTKH()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
EnsureVTKH::~EnsureVTKH()
{
// empty
}

//-----------------------------------------------------------------------------
void 
EnsureVTKH::declare_interface(Node &i)
{
    i["type_name"]   = "ensure_vtkh";
    i["port_names"].append() = "in";
    i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
void 
EnsureVTKH::execute()
{
    if(input(0).check_type<Node>())
    {
        // convert from blueprint to vtk-h
        const Node *n_input = input<Node>(0);
        vtkh::DataSet *res = DataAdapter::BlueprintToVTKHDataSet(*n_input);
        set_output<vtkh::DataSet>(res);

    }
    else if(input(0).check_type<vtkm::cont::DataSet>())
    {
        // wrap our vtk-m dataset in vtk-h
        vtkh::DataSet *res = DataAdapter::VTKmDataSetToVTKHDataSet(input<vtkm::cont::DataSet>(0));
        set_output<vtkh::DataSet>(res);
    }
    else if(input(0).check_type<vtkh::DataSet>())
    {
        // our data is already vtkh, pass though
        set_output(input(0));
    }
    else
    {
        ALPINE_ERROR("ensure_vtkh input must be a mesh blueprint "
                     "conforming conduit::Node, a vtk-m dataset, or vtk-h dataset");
    }
}


//-----------------------------------------------------------------------------
VTKHVolumeTracer::VTKHVolumeTracer()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
VTKHVolumeTracer::~VTKHVolumeTracer()
{
// empty
}

//-----------------------------------------------------------------------------
void 
VTKHVolumeTracer::declare_interface(Node &i)
{
    i["type_name"]   = "vtkh_volumetracer";
    i["port_names"].append() = "in";
    i["port_names"].append() = "renders";
    i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
VTKHVolumeTracer::verify_params(const conduit::Node &params,
                                   conduit::Node &info)
{
    info.reset();   
    bool res = true;
    
    if(! params.has_child("field") || 
       ! params["field"].dtype().is_string() )
    {
        info["errors"].append() = "Missing required string parameter 'field'";
    }
    return res;
}


//-----------------------------------------------------------------------------
void 
VTKHVolumeTracer::execute()
{
    if(!input(0).check_type<vtkh::DataSet>())
    {
        ALPINE_ERROR("vtkh_volumetracer input0 must be a vtk-h dataset");
    }
    if(!input(1).check_type<std::vector<vtkh::Render>>())
    {
        ALPINE_ERROR("vtkh_volumeracer input1 must be a vth-h render");
    }
 
    ALPINE_INFO("Doing the render!");
    bool composite = true;
    //
    // there is no need to check for compositing param
    // since a volume plot will always be at the end of 
    // a series of plots
    //
    vtkh::DataSet *data = input<vtkh::DataSet>(0);
    std::vector<vtkh::Render> *renders = input<std::vector<vtkh::Render>>(1);
    vtkh::VolumeRenderer tracer;  
    tracer.SetInput(data);
    tracer.SetDoComposite(composite);
    tracer.SetRenders(*renders);
    tracer.SetField(params()["field"].as_string());
    tracer.Update();
    
    std::vector<vtkh::Render> out_renders = tracer.GetRenders();
    //
    // We need to create a new pointer for the output because the input will be deleted
    // There is only a small amount of overhead since the canvases contained 
    // in the render will be shallow copied.
    //
    std::vector<vtkh::Render> *renders_ptr = new std::vector<vtkh::Render>();
    *renders_ptr = out_renders;
    set_output<std::vector<vtkh::Render>>(renders_ptr);
}

VTKHRayTracer::VTKHRayTracer()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
VTKHRayTracer::~VTKHRayTracer()
{
// empty
}

//-----------------------------------------------------------------------------
void 
VTKHRayTracer::declare_interface(Node &i)
{
    i["type_name"]   = "vtkh_raytracer";
    i["port_names"].append() = "in";
    i["port_names"].append() = "renders";
    i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
VTKHRayTracer::verify_params(const conduit::Node &params,
                                   conduit::Node &info)
{
    info.reset();   
    bool res = true;
    
    if(! params.has_child("field") || 
       ! params["field"].dtype().is_string() )
    {
        info["errors"].append() = "Missing required string parameter 'field'";
    }
    return res;
}


//-----------------------------------------------------------------------------
void 
VTKHRayTracer::execute()
{
    if(!input(0).check_type<vtkh::DataSet>())
    {
        ALPINE_ERROR("vtkh_raytracer input0 must be a vtk-h dataset");
    }
    if(!input(1).check_type<std::vector<vtkh::Render>>())
    {
        ALPINE_ERROR("vtkh_raytracer input1 must be a vth-h render");
    }
 
    ALPINE_INFO("Doing the render!");
    bool composite = true;
    if(params().has_path("composite"))
    {
      if(params()["composite"].as_string() == "false")
      {
        composite = false; 
      }
    }    

    vtkh::DataSet *data = input<vtkh::DataSet>(0);
    std::vector<vtkh::Render> *renders = input<std::vector<vtkh::Render>>(1);
    vtkh::RayTracer ray_tracer;  
    ray_tracer.SetInput(data);
    ray_tracer.SetDoComposite(composite);
    ray_tracer.SetRenders(*renders);
    ray_tracer.SetField(params()["field"].as_string());
    std::cout<<"PARAM "<<params()["field"].as_string()<<"\n";
    ray_tracer.Update();
    
    std::vector<vtkh::Render> out_renders = ray_tracer.GetRenders();
    //
    // We need to create a new pointer for the output because the input will be deleted
    // There is only a small amount of overhead since the canvases contained 
    // in the render will be shallow copied.
    //
    std::vector<vtkh::Render> *renders_ptr = new std::vector<vtkh::Render>();
    *renders_ptr = out_renders;
    set_output<std::vector<vtkh::Render>>(renders_ptr);
}


//-----------------------------------------------------------------------------
VTKHMarchingCubes::VTKHMarchingCubes()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
VTKHMarchingCubes::~VTKHMarchingCubes()
{
// empty
}

//-----------------------------------------------------------------------------
void 
VTKHMarchingCubes::declare_interface(Node &i)
{
    i["type_name"]   = "vtkh_marchingcubes";
    i["port_names"].append() = "in";
    i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
VTKHMarchingCubes::verify_params(const conduit::Node &params,
                                 conduit::Node &info)
{
    info.reset();
    bool res = true;
    
    if(! params.has_child("field") || 
       ! params["field"].dtype().is_string() )
    {
        info["errors"].append() = "Missing required string parameter 'field'";
    }
    
    if(! params.has_child("iso_values") || 
       ! params["iso_values"].dtype().is_number() )
    {
        info["errors"].append() = "Missing required numeric parameter 'iso_values'";
    }
    
    return res;
}


//-----------------------------------------------------------------------------
void 
VTKHMarchingCubes::execute()
{

    ALPINE_INFO("Marching the cubes!");
    params().print(); 
    if(!input(0).check_type<vtkh::DataSet>())
    {
        ALPINE_ERROR("vtkh_marchingcubes input must be a vtk-h dataset");
    }

    std::string field_name = params()["field"].as_string();
    
    vtkh::DataSet *data = input<vtkh::DataSet>(0);
    vtkh::MarchingCubes marcher;
    
    marcher.SetInput(data);
    marcher.SetField(field_name);

    const Node &n_iso_vals = params()["iso_values"];

    // convert to contig doubles
    Node n_iso_vals_dbls;
    n_iso_vals.to_float64_array(n_iso_vals_dbls);
    
    marcher.SetIsoValues(n_iso_vals.as_double_ptr(),
                         n_iso_vals.dtype().number_of_elements());

    marcher.Update();

    vtkh::DataSet *iso_output = marcher.GetOutput();
    
    set_output<vtkh::DataSet>(iso_output);
}

//-----------------------------------------------------------------------------
VTKHThreshold::VTKHThreshold()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
VTKHThreshold::~VTKHThreshold()
{
// empty
}

//-----------------------------------------------------------------------------
void 
VTKHThreshold::declare_interface(Node &i)
{
    i["type_name"]   = "vtkh_threshold";
    i["port_names"].append() = "in";
    i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
VTKHThreshold::verify_params(const conduit::Node &params,
                             conduit::Node &info)
{
    info.reset();
    bool res = true;
    
    if(! params.has_child("field") || 
       ! params["field"].dtype().is_string() )
    {
        info["errors"].append() = "Missing required string parameter 'field'";
    }
    
    if(! params.has_child("") || 
       ! params["min_value"].dtype().is_number() )
    {
        info["errors"].append() = "Missing required numeric parameter 'min_value'";
    }
    if(! params.has_child("") || 
       ! params["max_value"].dtype().is_number() )
    {
        info["errors"].append() = "Missing required numeric parameter 'max_value'";
    }
    
    return res;
}


//-----------------------------------------------------------------------------
void 
VTKHThreshold::execute()
{

    ALPINE_INFO("Thresholding!");
    
    if(!input(0).check_type<vtkh::DataSet>())
    {
        ALPINE_ERROR("VTKHThresholds input must be a vtk-h dataset");
    }

    std::string field_name = params()["field"].as_string();
    
    vtkh::DataSet *data = input<vtkh::DataSet>(0);
    vtkh::Threshold thresher;
    
    thresher.SetInput(data);
    thresher.SetField(field_name);

    const Node &n_min_val = params()["min_value"];
    const Node &n_max_val = params()["max_value"];

    // convert to contig doubles
    double min_val = n_min_val.as_float64(); 
    double max_val = n_max_val.as_float64(); 
    thresher.SetUpperThreshold(max_val);
    thresher.SetLowerThreshold(min_val);

    thresher.AddMapField(field_name);
    thresher.Update();

    vtkh::DataSet *thresh_output = thresher.GetOutput();
    
    set_output<vtkh::DataSet>(thresh_output);
}


//-----------------------------------------------------------------------------
DefaultRender::DefaultRender()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
DefaultRender::~DefaultRender()
{
// empty
}

//-----------------------------------------------------------------------------
void 
DefaultRender::declare_interface(Node &i)
{
    i["type_name"] = "default_render";
    i["port_names"].append() = "a";
    i["port_names"].append() = "b";
    i["port_names"].append() = "c";
    i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
DefaultRender::verify_params(const conduit::Node &params,
                             conduit::Node &info)
{
    info.reset();
    bool res = true;
    
    if(! params.has_child("pipeline_count")) 
    {
        info["errors"].append() = "Missing required numeric parameter 'pipelines'";
    }
  
    return res;
}

static int s_image_count = 0;
//-----------------------------------------------------------------------------
void 
DefaultRender::execute()
{

    ALPINE_INFO("We be default rendering!");
    
      
    const int count = params()["pipeline_count"].as_int32();
    vtkm::Bounds bounds;
    std::vector<vtkm::Id> domain_ids;
    vtkm::Id largest_dom_count = 0;
    for(int i = 0; i < count; ++i)
    {
      if(!input(i).check_type<vtkh::DataSet>())
      {
          ALPINE_ERROR("All inputs must be a vtk-h dataset");
      }
      vtkh::DataSet *data = input<vtkh::DataSet>(0);
      bounds.Include(data->GetGlobalBounds()); 
      //
      // we need to create one canvas for each domain.
      // Since filters can create empty data sets, we 
      // need to keep track of the most "complete"
      // data set
      //
      vtkm::Id dom_count = data->GetGlobalNumberOfDomains();
      if(largest_dom_count < dom_count)
      {
        domain_ids = data->GetDomainIds();
        largest_dom_count = dom_count;
      }
    }
    std::stringstream ss;
    ss<<"default_image_"<<s_image_count;
    s_image_count++;
    std::vector<vtkh::Render> *renders = new std::vector<vtkh::Render>();
    vtkh::Render render = vtkh::MakeRender<vtkh::RayTracer>(1024,
                                                            1024, 
                                                            bounds,
                                                            domain_ids,
                                                            ss.str());

    renders->push_back(render); 
    set_output<std::vector<vtkh::Render>>(renders);
}

//-----------------------------------------------------------------------------
VTKHClip::VTKHClip()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
VTKHClip::~VTKHClip()
{
// empty
}

//-----------------------------------------------------------------------------
void 
VTKHClip::declare_interface(Node &i)
{
    i["type_name"] = "vtkh_clip";
    i["port_names"].append() = "in";
    i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
VTKHClip::verify_params(const conduit::Node &params,
                             conduit::Node &info)
{
    info.reset();
    bool res = true;
    
    if(! params.has_child("") || 
       ! params["sphere"].dtype().is_number() )
    {
        info["errors"].append() = "Missing required numeric parameter 'sphere'";
    }
    
    // TODO: check for other clip types 
    return res;
}


//-----------------------------------------------------------------------------
void 
VTKHClip::execute()
{

    ALPINE_INFO("We be clipping!");
    
    if(!input(0).check_type<vtkh::DataSet>())
    {
        ALPINE_ERROR("VTKHClip input must be a vtk-h dataset");
    }

    
    vtkh::DataSet *data = input<vtkh::DataSet>(0);
    vtkh::Clip clipper;
    
    clipper.SetInput(data);

    if(params().has_child("topology"))
    {
      std::string topology = params()["topology"].as_string();
      clipper.SetCellSet(topology);
    }

    const Node &sphere = params()["sphere"];
    double center[3];

    center[0] = sphere["center/x"].as_float64();
    center[1] = sphere["center/y"].as_float64();
    center[2] = sphere["center/z"].as_float64();
    double radius = sphere["radius"].as_float64(); 
  
    clipper.SetSphereClip(center, radius);
    clipper.Update();

    vtkh::DataSet *clip_output = clipper.GetOutput();
    
    set_output<vtkh::DataSet>(clip_output);
}

//-----------------------------------------------------------------------------
Alias::Alias()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
Alias::~Alias()
{
// empty
}

//-----------------------------------------------------------------------------
void 
Alias::declare_interface(Node &i)
{
    i["type_name"]   = "alias";
    i["port_names"].append() = "in";
    i["output_port"] = "true";
}


//-----------------------------------------------------------------------------
void 
Alias::execute()
{
    set_output(input(0));
}


//-----------------------------------------------------------------------------
EnsureVTKM::EnsureVTKM()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
EnsureVTKM::~EnsureVTKM()
{
// empty
}

//-----------------------------------------------------------------------------
void 
EnsureVTKM::declare_interface(Node &i)
{
    i["type_name"]   = "ensure_vtkm";
    i["port_names"].append() = "in";
    i["output_port"] = "true";
}


//-----------------------------------------------------------------------------
void 
EnsureVTKM::execute()
{
#if !defined(ALPINE_VTKM_ENABLED)
        ALPINE_ERROR("alpine was not built with VTKm support!");
#else
    if(input(0).check_type<vtkm::cont::DataSet>())
    {
        set_output(input(0));
    }
    else if(input(0).check_type<Node>())
    {
        // convert from conduit to vtkm
        const Node *n_input = input<Node>(0);
        vtkm::cont::DataSet  *res = DataAdapter::BlueprintToVTKmDataSet(*n_input);
        set_output<vtkm::cont::DataSet>(res);
    }
    else
    {
        ALPINE_ERROR("unsupported input type for ensure_vtkm");
    }
#endif
}



//-----------------------------------------------------------------------------
};
//-----------------------------------------------------------------------------
// -- end alpine::runtime::filters --
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
};
//-----------------------------------------------------------------------------
// -- end alpine::runtime --
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
};
//-----------------------------------------------------------------------------
// -- end alpine:: --
//-----------------------------------------------------------------------------





