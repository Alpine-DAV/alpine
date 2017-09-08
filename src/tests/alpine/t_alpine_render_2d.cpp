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
/// file: t_alpine_render_2d.cpp
///
//-----------------------------------------------------------------------------

#include "gtest/gtest.h"

#include <alpine.hpp>

#include <iostream>
#include <math.h>

#include <conduit_blueprint.hpp>

#include "t_config.hpp"
#include "t_utils.hpp"



using namespace std;
using namespace conduit;
using namespace alpine;

index_t EXAMPLE_MESH_SIDE_DIM = 50;

//-----------------------------------------------------------------------------
TEST(alpine_render_2d, test_render_2d_default_runtime)
{
    // the vtkm runtime is currently our only rendering runtime
    Node n;
    alpine::about(n);
    // only run this test if alpine was built with vtkm support
    if(n["runtimes/ascent/status"].as_string() == "disabled")
    {
        ALPINE_INFO("Ascent support disabled, skipping 2D default"
                      "Pipeline test");

        return;
    }
    
    //
    // Create example mesh.
    //
    Node data, verify_info;
    conduit::blueprint::mesh::examples::braid("quads",
                                               EXAMPLE_MESH_SIDE_DIM,
                                               EXAMPLE_MESH_SIDE_DIM,
                                               0,
                                               data);
    
    EXPECT_TRUE(conduit::blueprint::mesh::verify(data,verify_info));
    verify_info.print();
    string output_path = prepare_output_dir();
    string output_file = conduit::utils::join_file_path(output_path, "tout_render_2d_default_runtime");
    // remove old images before rendering
    remove_test_image(output_file);

    //
    // Create the actions.
    //
    Node actions;

    conduit::Node scenes;
    scenes["scene1/plots/plt1/type"]         = "pseudocolor";
    scenes["scene1/plots/plt1/params/field"] = "braid";
    scenes["scene1/image_prefix"] = output_file;

    conduit::Node &add_scenes = actions.append();
    add_scenes["action"] = "add_scenes";
    add_scenes["scenes"] = scenes;
    conduit::Node &execute = actions.append();
    execute["action"] = "execute";
    actions.print();
    
    //
    // Run Alpine
    //
    
    Alpine alpine;
    Node alpine_opts;
    // default is now alpine
    alpine_opts["runtime/type"] = "ascent";
    alpine.open(alpine_opts);
    alpine.publish(data);
    alpine.execute(actions);
    alpine.close();

    // check that we created an image
    EXPECT_TRUE(check_test_image(output_file));
}
//-----------------------------------------------------------------------------
TEST(alpine_render_2d, test_render_2d_render_serial_backend)
{
    
    Node n;
    alpine::about(n);
    // only run this test if alpine was built with vtkm support
    if(n["runtimes/ascent/status"].as_string() == "disabled")
    {
        ALPINE_INFO("Ascent support disabled, skipping 2D Ascent Serial "
                      "Pipeline test");
        return;
    }
    
    ALPINE_INFO("Testing 2D Ascent Runtime");
    
    //
    // Create an example mesh.
    //
    Node data, verify_info;
    conduit::blueprint::mesh::examples::braid("quads",
                                               EXAMPLE_MESH_SIDE_DIM,
                                               EXAMPLE_MESH_SIDE_DIM,
                                               0,
                                               data);
    //conduit::blueprint::mesh::examples::braid("hexs",100,100,100,data);
    
    EXPECT_TRUE(conduit::blueprint::mesh::verify(data,verify_info));
    verify_info.print();
    
    string output_path = prepare_output_dir();
    string output_file = conduit::utils::join_file_path(output_path, "tout_render_2d_ascent_serial_backend");
    // remove old images before rendering
    remove_test_image(output_file);

    //
    // Create the actions.
    //
    Node actions;

    conduit::Node scenes;
    scenes["scene1/plots/plt1/type"]         = "pseudocolor";
    scenes["scene1/plots/plt1/params/field"] = "braid";
    scenes["scene1/image_prefix"] =  output_file;

    conduit::Node &add_scenes = actions.append();
    add_scenes["action"] = "add_scenes";
    add_scenes["scenes"] = scenes;
    conduit::Node &execute = actions.append();
    execute["action"] = "execute";
    actions.print();
    
    //
    // Run Alpine
    //
    
    Alpine alpine;
    Node alpine_opts;
    // default is now alpine
    alpine_opts["runtime/type"] = "ascent";
    alpine_opts["runtime/backend"] = "serial";
    alpine.open(alpine_opts);
    alpine.publish(data);
    alpine.execute(actions);
    alpine.close();

    // check that we created an image
    EXPECT_TRUE(check_test_image(output_file));
}

//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    int result = 0;

    ::testing::InitGoogleTest(&argc, argv);
    
    // allow override of the data size via the command line
    if(argc == 2)
    { 
        EXAMPLE_MESH_SIDE_DIM = atoi(argv[1]);
    }
    
    result = RUN_ALL_TESTS();
    return result;
}


