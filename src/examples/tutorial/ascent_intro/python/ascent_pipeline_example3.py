###############################################################################
# Copyright (c) 2015-2019, Lawrence Livermore National Security, LLC.
#
# Produced at the Lawrence Livermore National Laboratory
#
# LLNL-CODE-716457
#
# All rights reserved.
#
# This file is part of Ascent.
#
# For details, see: http://ascent.readthedocs.io/.
#
# Please also read ascent/LICENSE
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice,
#   this list of conditions and the disclaimer below.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the disclaimer (as noted below) in the
#   documentation and/or other materials provided with the distribution.
#
# * Neither the name of the LLNS/LLNL nor the names of its contributors may
#   be used to endorse or promote products derived from this software without
#   specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL LAWRENCE LIVERMORE NATIONAL SECURITY,
# LLC, THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
# IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
###############################################################################


import conduit
import conduit.blueprint
import ascent
import numpy as np


mesh = conduit.Node()
conduit.blueprint.mesh.examples.braid("hexs",
                                      25,
                                      25,
                                      25,
                                      mesh)

# Use Ascent to create and render multiple pipelines
a = ascent.Ascent()
a.open()

# publish mesh to ascent
a.publish(mesh);

# setup actions
actions = conduit.Node()
add_act = actions.append()
add_act["action"] = "add_pipelines"
pipelines = add_act["pipelines"]

# create our first pipeline (pl1) 
# with a contour filter (f1)
pipelines["pl1/f1/type"] = "contour"
# extract contours where braid variable
# equals 0.2 and 0.4
contour_params = pipelines["pl1/f1/params"]
contour_params["field"] = "braid"
iso_vals = np.array([0.2, 0.4],dtype=np.float32)
contour_params["iso_values"].set(iso_vals)

# create our second pipeline (pl2) with a threshold filter (f1)
# and a clip filter (f2)

# add our threshold (pl2 f1)
pipelines["pl2/f1/type"] = "threshold"
thresh_params = pipelines["pl2/f1/params"]
# set threshold parameters
# keep elements with values between 0.0 and 0.5
thresh_params["field"]  = "braid"
thresh_params["min_value"] = 0.0
thresh_params["max_value"] = 0.5


# add our clip (pl2 f2)
pipelines["pl2/f2/type"]   = "clip"
clip_params = pipelines["pl2/f2/params"]
# set clip parameters
# use spherical clip
clip_params["sphere/center/x"] = 0.0
clip_params["sphere/center/y"] = 0.0
clip_params["sphere/center/z"] = 0.0
clip_params["sphere/radius"]   = 12

# declare a scene to render our pipeline results
add_act2 = actions.append()
add_act2["action"] = "add_scenes"
scenes = add_act2["scenes"]

# add a scene (s1) with two pseudocolor plots 
# (p1 and p2) that will render the results 
# of our pipelines (pl1 and pl2)## Pipeline Example 2:

# plot (p1) to render our first pipeline (pl1)
scenes["s1/plots/p1/type"] = "pseudocolor"
scenes["s1/plots/p1/pipeline"] = "pl1"
scenes["s1/plots/p1/field"] = "braid"
# plot (p2) to render our second pipeline (pl2)
scenes["s1/plots/p2/type"] = "pseudocolor"
scenes["s1/plots/p2/pipeline"] = "pl2"
scenes["s1/plots/p2/field"] = "braid"
# set the output file name (ascent will add ".png")
scenes["s1/image_name"] = "out_pipeline_ex3_two_plots"

# print our full actions tree
print(actions.to_yaml())

# execute
a.execute(actions)

# close ascent
a.close()
