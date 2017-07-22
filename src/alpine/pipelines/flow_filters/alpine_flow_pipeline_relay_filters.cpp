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
/// file: alpine_flow_pipeline_relay_filters.cpp
///
//-----------------------------------------------------------------------------

#include "alpine_flow_pipeline_relay_filters.hpp"

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
#include <alpine_file_system.hpp>
#include <alpine_flow_graph.hpp>
#include <alpine_flow_workspace.hpp>
#include <alpine_file_system.hpp>

// mpi related includes
#ifdef PARALLEL
#include <mpi.h>
// -- conduit relay mpi
#include <conduit_relay_mpi.hpp>
#endif


// mpi related includes
#ifdef PARALLEL
#include <mpi.h>
// -- conduit relay mpi
#include <conduit_relay_mpi.hpp>
#endif

using namespace std;
using namespace conduit;
using namespace conduit::relay;
using namespace alpine::flow;

//-----------------------------------------------------------------------------
// -- begin alpine:: --
//-----------------------------------------------------------------------------
namespace alpine
{

//-----------------------------------------------------------------------------
// -- begin alpine::pipeline --
//-----------------------------------------------------------------------------
namespace pipeline
{

//-----------------------------------------------------------------------------
// -- begin alpine::pipeline::flow --
//-----------------------------------------------------------------------------
namespace flow
{

//-----------------------------------------------------------------------------
// -- begin alpine::pipeline::flow::filters --
//-----------------------------------------------------------------------------
namespace filters
{



//-----------------------------------------------------------------------------
// helper shared by io save and load
//-----------------------------------------------------------------------------
bool
verify_io_params(const conduit::Node &params,
                 conduit::Node &info)
{
    bool res = true;
    
    if( !params.has_child("path") ) 
    {
        info["errors"].append() = "missing required entry 'path'";
        res = false;
    }
    else if(!params["path"].dtype().is_string())
    {
        info["errors"].append() = "'path' must be a string";
        res = false;
    }
    else if(params["path"].as_string().empty())
    {
        info["errors"].append() = "'path' is an empty string";
        res = false;
    }
    
    
    if( params.has_child("protocol") ) 
    {
        if(!params["protocol"].dtype().is_string())
        {
            info["errors"].append() = "optional entry 'protocol' must be a string";
            res = false;
        }
        else if(params["protocol"].as_string().empty())
        {
            info["errors"].append() = "'protocol' is an empty string";
            res = false;
        }
        else
        {
            info["info"].append() = "includes 'protocol'";
        }
    }

    return res;
}

//-----------------------------------------------------------------------------
void mesh_blueprint_save(const Node &data,
                         const std::string &path,
                         const std::string &file_protocol)
{
    int par_rank = 0;  
    int par_size = 1;
    
#ifdef PARALLEL
    MPI_Comm mpi_comm = MPI_Comm_f2c(Workspace::default_mpi_comm());
    MPI_Comm_rank(mpi_comm, &par_rank);
    MPI_Comm_size(mpi_comm, &par_size);
#endif
    
    // get cycle and domain id from the mesh
    uint64 domain = data["state/domain_id"].to_value();
    uint64 cycle  = data["state/cycle"].to_value();

    char fmt_buff[64];
    snprintf(fmt_buff, sizeof(fmt_buff), "%06lu",cycle);
    
    std::string output_base_path = path;
    
    ostringstream oss;
    oss << output_base_path << ".cycle_" << fmt_buff;
    string output_dir  =  oss.str();
    
    snprintf(fmt_buff, sizeof(fmt_buff), "%06lu",domain);
    oss.str("");
    oss << "domain_" << fmt_buff << "." << file_protocol;
    string output_file  = conduit::utils::join_file_path(output_dir,oss.str());


    bool dir_ok = false;

    // let rank zero handle dir creation
    if(par_rank == 0)
    {
        // check of the dir exists
        dir_ok = directory_exists(output_dir);
        if(!dir_ok)
        {
            // if not try to let rank zero create it
            dir_ok = create_directory(output_dir);
        }
    }
    
    
    int num_domains = 1;
#ifdef PARALLEL
    // use barrier to wait until dir is created
    MPI_Barrier(mpi_comm);

    // TODO: Support domain overloaded.. 
    num_domains = par_size;

    // TODO:
    // This a reduce to check for an error ... 
    // it will be a common pattern, how do we make this easy?
    
    // use an mpi sum to check if the dir exists
    Node n_src, n_reduce;
    
    if(dir_ok)
        n_src = (int)1;
    else
        n_src = (int)0;

    mpi::all_reduce(n_src,
                    n_reduce,
                    MPI_INT,
                    MPI_MAX,
                    mpi_comm);
    dir_ok = (n_reduce.as_int() == 1);
#endif

    if(!dir_ok)
    {
        ALPINE_ERROR("Error: failed to create directory " << output_dir);
    }


    relay::io::save(data,output_file);

    // let rank zero write out the root file
    if(par_rank == 0)
    {
        snprintf(fmt_buff, sizeof(fmt_buff), "%06lu",cycle);

        oss.str("");
        oss << path
            << ".cycle_" 
            << fmt_buff 
            << ".root";

        string root_file = oss.str();

        string output_dir_base, output_dir_path;

        // TODO: Fix for windows
        conduit::utils::rsplit_string(output_dir,
                                      "/",
                                      output_dir_base,
                                      output_dir_path);

        string output_file_pattern = conduit::utils::join_file_path(output_dir_base,
                                                                    "domain_%06d." + file_protocol);


        Node root;
        Node &bp_idx = root["blueprint_index"];

        blueprint::mesh::generate_index(data,
                                        "",
                                        num_domains,
                                        bp_idx["mesh"]);
            
        root["protocol/name"]    = "conduit_" + file_protocol;
        root["protocol/version"] = "0.2.1";

        root["number_of_files"]  = num_domains;
        root["number_of_trees"]  = num_domains;
        // TODO: make sure this is relative 
        root["file_pattern"]     = output_file_pattern;
        root["tree_pattern"]     = "/";
        relay::io::save(root,root_file,file_protocol);
    }
}



//-----------------------------------------------------------------------------
RelayIOSave::RelayIOSave()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
RelayIOSave::~RelayIOSave()
{
// empty
}

//-----------------------------------------------------------------------------
void 
RelayIOSave::declare_interface(Node &i)
{
    i["type_name"]   = "relay_io_save";
    i["port_names"].append() = "in";
    i["output_port"] = "false";
}

//-----------------------------------------------------------------------------
bool
RelayIOSave::verify_params(const conduit::Node &params,
                           conduit::Node &info)
{
    return verify_io_params(params,info);
}


//-----------------------------------------------------------------------------
void 
RelayIOSave::execute()
{
    std::string path, protocol;
    path = params()["path"].as_string();
    
    // TODO check if we need to expand the path (MPI) case for std protocols
    if(params().has_child("protocol"))
    {
        protocol = params()["protocol"].as_string();
    }

    if(!input("in").check_type<Node>())
    {
        // error
        ALPINE_ERROR("relay_io_save requires a conduit::Node input");
    }
    
    Node *in = input<Node>("in");
    
    if(protocol.empty())
    {
        conduit::relay::io::save(*in,path);
    }
    else if( protocol == "blueprint/mesh/hdf5")
    {
        mesh_blueprint_save(*in,path,"hdf5");
    }
    else
    {
        conduit::relay::io::save(*in,path,protocol);
    }

}



//-----------------------------------------------------------------------------
RelayIOLoad::RelayIOLoad()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
RelayIOLoad::~RelayIOLoad()
{
// empty
}

//-----------------------------------------------------------------------------
void 
RelayIOLoad::declare_interface(Node &i)
{
    i["type_name"]   = "relay_io_load";
    i["port_names"] = DataType::empty();
    i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
RelayIOLoad::verify_params(const conduit::Node &params,
                           conduit::Node &info)
{
    return verify_io_params(params,info);
}


//-----------------------------------------------------------------------------
void 
RelayIOLoad::execute()
{
    std::string path, protocol;
    path = params()["path"].as_string();
    
    // TODO check if we need to expand the path (MPI) case
    if(params().has_child("protocol"))
    {
        protocol = params()["protocol"].as_string();
    }

    Node *res = new Node();
    
    if(protocol.empty())
    {
        conduit::relay::io::load(path,*res);
    }
    else
    {
        conduit::relay::io::load(path,protocol,*res);
    }
    
    set_output<Node>(res);

}


//-----------------------------------------------------------------------------
};
//-----------------------------------------------------------------------------
// -- end alpine::pipeline::flow::filters --
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
};
//-----------------------------------------------------------------------------
// -- end alpine::pipeline::flow --
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
};
//-----------------------------------------------------------------------------
// -- end alpine::pipeline --
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
};
//-----------------------------------------------------------------------------
// -- end alpine:: --
//-----------------------------------------------------------------------------





