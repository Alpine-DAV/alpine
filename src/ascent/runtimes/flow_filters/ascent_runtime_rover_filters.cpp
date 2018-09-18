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
/// file: ascent_runtime_rover_filters.cpp
///
//-----------------------------------------------------------------------------

#include "ascent_runtime_rover_filters.hpp"

//-----------------------------------------------------------------------------
// thirdparty includes
//-----------------------------------------------------------------------------

// conduit includes
#include <conduit.hpp>
#include <conduit_relay.hpp>
#include <conduit_blueprint.hpp>

//-----------------------------------------------------------------------------
// ascent includes
//-----------------------------------------------------------------------------
#include <ascent_logging.hpp>
#include <flow_graph.hpp>
#include <flow_workspace.hpp>

// mpi
#ifdef ASCENT_MPI_ENABLED
#include <mpi.h>
#endif

#if defined(ASCENT_VTKM_ENABLED)
#include <rover.hpp>
#include <ray_generators/camera_generator.hpp>
#include <vtkh/vtkh.hpp>
#include <vtkh/DataSet.hpp>
#include <ascent_vtkh_data_adapter.hpp>
#endif

using namespace conduit;
using namespace std;

using namespace flow;
using namespace rover;

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
// -- begin ascent::runtime::filters --
//-----------------------------------------------------------------------------
namespace filters
{
;

//-----------------------------------------------------------------------------
RoverXRay::RoverXRay()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
RoverXRay::~RoverXRay()
{
// empty
}

//-----------------------------------------------------------------------------
void 
RoverXRay::declare_interface(Node &i)
{
    i["type_name"]   = "xray";
    i["port_names"].append() = "in";
    i["output_port"] = "false";
}

//-----------------------------------------------------------------------------
bool
RoverXRay::verify_params(const conduit::Node &params,
                                 conduit::Node &info)
{
    info.reset();
    bool res = true;
    
    if(! params.has_child("absorption") || 
       ! params["absorption"].dtype().is_string() )
    {
        info["errors"].append() = "Missing required string parameter 'absorption'";
        res = false;
    }

    if(! params.has_child("filename") || 
       ! params["filename"].dtype().is_string() )
    {
        info["errors"].append() = "Missing required string parameter 'filename'";
        res = false;
    }

    if( params.has_child("emission") &&
       ! params["emission"].dtype().is_string() )
    {
        info["errors"].append() = "Optional parameter 'emission' must be a string";
        res = false;
    }

    if( params.has_child("precision") &&
       ! params["precision"].dtype().is_string() )
    {
        info["errors"].append() = "Optional parameter 'precision' must be a string";
        std::string prec = params["precision"].as_string();
        if(prec != "single" || prec != "double")
        {
          info["errors"].append() = "Parameter 'precision' must be 'single' or 'double'";
        }
        res = false;
    }

    return res;
}

//-----------------------------------------------------------------------------
void 
RoverXRay::execute()
{

    ASCENT_INFO("XRay sees everything!");
    vtkh::DataSet *dataset = nullptr;

    if(input(0).check_type<Node>())
    {
        // convert from blueprint to vtk-h
        const Node *n_input = input<Node>(0);
        dataset = VTKHDataAdapter::BlueprintToVTKHDataSet(*n_input);
    }
    
    vtkmCamera camera;
    camera.ResetToBounds(dataset->GetGlobalBounds());

    CameraGenerator generator(camera, 512, 512);

    Rover tracer;
#ifdef ASCENT_MPI_ENABLED
    int comm_id =flow::Workspace::default_mpi_comm();
    tracer.set_mpi_comm_handle(comm_id);
#endif
    
    if(params().has_path("precision"))
    {
      std::string prec = params()["precision"].as_string();
      if(prec == "double")
      {
        tracer.set_tracer_precision64();
      }
    }
    
    //
    // Create some basic settings 
    //
    RenderSettings settings;
    settings.m_primary_field = params()["absorption"].as_string();

    if(params().has_path("emission"))
    {
       settings.m_secondary_field = params()["emission"].as_string();
    }
  
    
    settings.m_render_mode = rover::energy;
    //settings.m_render_mode = rover::volume;
    //vtkmColorTable color_table("cool to warm");
    //color_table.AddPointAlpha(0.0, .01);
    //color_table.AddPointAlpha(0.5, .02);
    //color_table.AddPointAlpha(1.0, .01);
    //settings.m_color_table = color_table;

    tracer.set_render_settings(settings);
    for(int i = 0; i < dataset->GetNumberOfDomains(); ++i)
    {
      //dataset->GetDomain(i).PrintSummary(std::cout);
      tracer.add_data_set(dataset->GetDomain(i));
    }

    tracer.set_ray_generator(&generator);
    tracer.execute();

    std::string filename = params()["filename"].as_string();
    //tracer.save_png("rover");
    tracer.save_png(filename);
    tracer.finalize();

    delete dataset;
}

//-----------------------------------------------------------------------------
};
//-----------------------------------------------------------------------------
// -- end ascent::runtime::filters --
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





