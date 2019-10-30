.. ############################################################################
.. # Copyright (c) 2015-2019, Lawrence Livermore National Security, LLC.
.. #
.. # Produced at the Lawrence Livermore National Laboratory
.. #
.. # LLNL-CODE-716457
.. #
.. # All rights reserved.
.. #
.. # This file is part of Ascent.
.. #
.. # For details, see: http://ascent.readthedocs.io/.
.. #
.. # Please also read ascent/LICENSE
.. #
.. # Redistribution and use in source and binary forms, with or without
.. # modification, are permitted provided that the following conditions are met:
.. #
.. # * Redistributions of source code must retain the above copyright notice,
.. #   this list of conditions and the disclaimer below.
.. #
.. # * Redistributions in binary form must reproduce the above copyright notice,
.. #   this list of conditions and the disclaimer (as noted below) in the
.. #   documentation and/or other materials provided with the distribution.
.. #
.. # * Neither the name of the LLNS/LLNL nor the names of its contributors may
.. #   be used to endorse or promote products derived from this software without
.. #   specific prior written permission.
.. #
.. # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
.. # AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
.. # IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
.. # ARE DISCLAIMED. IN NO EVENT SHALL LAWRENCE LIVERMORE NATIONAL SECURITY,
.. # LLC, THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE LIABLE FOR ANY
.. # DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
.. # DAMAGES  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
.. # OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
.. # HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
.. # STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
.. # IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
.. # POSSIBILITY OF SUCH DAMAGE.
.. #
.. ############################################################################

.. _tutorial_intro:

Introduction to Ascent
=============================

.. _tutorial_first_light:

First Light
--------------

*Render a sample dataset using Ascent from C++ and Python*

For this demo, we run the "First Light" examples which are installed with Ascent to enable users to quickly test ascent in their build system.

C++ Example (`examples/ascent/using-with-make/ascent_render_example.cpp`):

.. literalinclude:: ../../../src/examples/using-with-make/ascent_render_example.cpp
   :language: cpp
   :lines: 45-

Python Example (`examples/ascent/python/ascent_python_render_example.py`):

.. literalinclude:: ../../../src/examples/python/ascent_python_render_example.py
   :language: python
   :lines: 45-


These examples render the same example data set using ray casting to create a pseudocolor plot. 
The data set is one of the built-in Conduit Mesh Blueprint examples, in this case an unstructured mesh composed of hexagons.


.. _tutorial_conduit_basics:

Conduit Basics
----------------

Ascent's API is based on Conduit (LINK). Both mesh data and action descriptions are passed to Ascent as Conduit trees.  The Conduit C++ and Python interfaces are very similar, with the C++ interface heavily influenced by the ease of use of Python. These examples provide basic knowledge about creating Conduit Nodes to use with Ascent.

Creating key-value entries
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:download:`C++ <../../../src/examples/tutorial/ascent_intro/cpp/conduit_example1.cpp>`

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/cpp/conduit_example1.cpp
   :language: cpp

:download:`Python <../../../src/examples/tutorial/ascent_intro/python/conduit_example1.py>`

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/python/conduit_example1.py
   :language: python


Creating a path hierarchy
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:download:`C++ <../../../src/examples/tutorial/ascent_intro/cpp/conduit_example2.cpp>`

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/cpp/conduit_example2.cpp
   :language: cpp

:download:`Python <../../../src/examples/tutorial/ascent_intro/python/conduit_example2.py>`

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/python/conduit_example2.py
   :language: python


Setting array data
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:download:`C++ <../../../src/examples/tutorial/ascent_intro/cpp/conduit_example3.cpp>`

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/cpp/conduit_example3.cpp
   :language: cpp

:download:`Python <../../../src/examples/tutorial/ascent_intro/python/conduit_example3.py>`

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/python/conduit_example3.py
   :language: python


Zero-copy vs deep copy of array data
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:download:`C++ <../../../src/examples/tutorial/ascent_intro/cpp/conduit_example4.cpp>`

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/cpp/conduit_example4.cpp
   :language: cpp

:download:`Python <../../../src/examples/tutorial/ascent_intro/python/conduit_example4.py>`

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/python/conduit_example4.py
   :language: python

You can also find more Conduit examples in Conduit's documentation (LINK)

Conduit Blueprint Mesh Examples
---------------------------------

Simulation mesh data is passed to Ascent using a shared set of conventions called the **Mesh Blueprint**.
(LINK) (MORE)


Creating a uniform grid with a single field
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**C++**

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/cpp/blueprint_example1.cpp
   :language: cpp

**Python**

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/python/blueprint_example1.py
   :language: python

(PICTURE OF THE RESULT)


Creating an unstructured tet mesh with fields
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**C++**

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/cpp/blueprint_example2.cpp
   :language: cpp

**Python**

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/python/blueprint_example2.py
   :language: python

(PICTURE OF THE RESULT)

Using the built-in blueprint  ``braid`` example mesh
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

(ALSO TALK ABOUT BUILT CONDUIT EXAMPLE FUNCTIONS)

(TODO) describe braid, and possibly tweak to make time varying

**C++**

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/cpp/blueprint_example3.cpp
   :language: cpp

**Python**

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/python/blueprint_example3.py
   :language: python


You can also find more Conduit examples in Conduit’s documentation (LINK)


Rendering images with Scenes
-------------------------------

SHORT BLURB ABOUT SCENES, link to docs etc

Using multiple scenes to render different variables 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:download:`C++ <../../../src/examples/tutorial/ascent_intro/cpp/ascent_scene_example1.cpp>`

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/cpp/ascent_scene_example1.cpp
   :language: cpp

:download:`Python <../../../src/examples/tutorial/ascent_intro/python/ascent_scene_example1.py>`

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/python/ascent_scene_example1.py
   :language: python


Rendering multiple plots to a single image
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:download:`C++ <../../../src/examples/tutorial/ascent_intro/cpp/ascent_scene_example2.cpp>`

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/cpp/ascent_scene_example2.cpp
   :language: cpp

:download:`Python <../../../src/examples/tutorial/ascent_intro/python/ascent_scene_example2.py>`

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/python/ascent_scene_example2.py
   :language: python


Rendering multiple views of the same scene
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:download:`C++ <../../../src/examples/tutorial/ascent_intro/cpp/ascent_scene_example3.cpp>`

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/cpp/ascent_scene_example3.cpp
   :language: cpp

:download:`Python <../../../src/examples/tutorial/ascent_intro/python/ascent_scene_example3.py>`

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/python/ascent_scene_example3.py
   :language: python


Changing the color table used for pseudocolor rendering
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:download:`C++ <../../../src/examples/tutorial/ascent_intro/cpp/ascent_scene_example4.cpp>`

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/cpp/ascent_scene_example4.cpp
   :language: cpp

:download:`Python <../../../src/examples/tutorial/ascent_intro/python/ascent_scene_example4.py>`

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/python/ascent_scene_example4.py
   :language: python


Transforming data with Pipelines
-----------------------------------

SHORT BLURB ABOUT PIPELINES, link to docs etc

Calculate and render contours
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:download:`C++ <../../../src/examples/tutorial/ascent_intro/cpp/ascent_pipeline_example1.cpp>`

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/cpp/ascent_pipeline_example1.cpp
   :language: cpp

:download:`Python <../../../src/examples/tutorial/ascent_intro/python/ascent_pipeline_example1.py>`

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/python/ascent_pipeline_example1.py
   :language: python


Combining threshold and clip transforms
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:download:`C++ <../../../src/examples/tutorial/ascent_intro/cpp/ascent_pipeline_example1.cpp>`

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/cpp/ascent_pipeline_example1.cpp
   :language: cpp

:download:`Python <../../../src/examples/tutorial/ascent_intro/python/ascent_pipeline_example1.py>`

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/python/ascent_pipeline_example1.py
   :language: python

Creating and rendering the multiple pipelines
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:download:`C++ <../../../src/examples/tutorial/ascent_intro/cpp/ascent_pipeline_example1.cpp>`

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/cpp/ascent_pipeline_example1.cpp
   :language: cpp

:download:`Python <../../../src/examples/tutorial/ascent_intro/python/ascent_pipeline_example1.py>`

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/python/ascent_pipeline_example1.py
   :language: python


Capturing data with Extracts
-----------------------------------

SHORT BLURB ABOUT EXTRACTS, link to docs etc

Exporting input mesh data to Blueprint HDF5 files
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:download:`C++ <../../../src/examples/tutorial/ascent_intro/cpp/ascent_pipeline_example1.cpp>`

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/cpp/ascent_pipeline_example1.cpp
   :language: cpp

:download:`Python <../../../src/examples/tutorial/ascent_intro/python/ascent_pipeline_example1.py>`

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/python/ascent_pipeline_example1.py
   :language: python



VisIt 2.13 or newer, when built with Conduit support, can load and visualize these files.


Exporting the result of a pipeline to Blueprint HDF5 files
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:download:`C++ <../../../src/examples/tutorial/ascent_intro/cpp/ascent_pipeline_example2.cpp>`

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/cpp/ascent_pipeline_example2.cpp
   :language: cpp

:download:`Python <../../../src/examples/tutorial/ascent_intro/python/ascent_pipeline_example2.py>`

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/python/ascent_pipeline_example2.py
   :language: python


Creating a Cinema image database for post-hoc exploration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:download:`C++ <../../../src/examples/tutorial/ascent_intro/cpp/ascent_extract_example2.cpp>`

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/cpp/ascent_extract_example2.cpp
   :language: cpp

:download:`Python <../../../src/examples/tutorial/ascent_intro/python/ascent_extract_example2.py>`

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/python/ascent_extract_example2.py
   :language: python


If you are already in Python land this may seem strange, but for C++ and FORTRAN codes, the Python extract gives you access to a distributed-memory python environment with in-situ access to mesh data


Using a Python Extract to execute custom Python analysis
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:download:`C++ <../../../src/examples/tutorial/ascent_intro/cpp/ascent_extract_example3.cpp>`

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/cpp/ascent_extract_example3.cpp
   :language: cpp

:download:`Python <../../../src/examples/tutorial/ascent_intro/python/ascent_extract_example3.py>`

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/python/ascent_extract_example3.py
   :language: python


If you are already in Python land this may seem strange, but for C++ and FORTRAN codes, the Python extract gives you access to a distributed-memory python environment with in-situ access to mesh data



Adaptive visualization with Triggers
--------------------------------------

SHORT BLURB ABOUT TRIGGERS, link to docs etc


Using a trigger to rendering when an event occurs
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:download:`C++ <../../../src/examples/tutorial/ascent_intro/cpp/ascent_tirgger_example1.cpp>`

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/cpp/ascent_tirgger_example1.cpp
   :language: cpp

:download:`Python <../../../src/examples/tutorial/ascent_intro/python/ascent_tirgger_example1.py>`

.. literalinclude:: ../../../src/examples/tutorial/ascent_intro/python/ascent_tirgger_example1.py
   :language: python


All Intro Examples
----------------------

Tutorial Notebooks
~~~~~~~~~~~~~~~~~~~

* First Light:  :download:`01_ascent_first_light.ipynb <../../../src/examples/tutorial/ascent_intro/notebooks/01_ascent_first_light.ipynb>`
* Conduit Basics:  :download:`02_conduit_basics.ipynb <../../../src/examples/tutorial/ascent_intro/notebooks/02_conduit_basics.ipynb>`
* Conduit Blueprint Mesh Examples: :download:`03_conduit_blueprint_mesh_examples.cpp <../../../src/examples/tutorial/ascent_intro/notebooks/03_conduit_blueprint_mesh_examples.ipynb>`
* Scene Examples:    :download:`04_ascent_scene_examples.cpp <../../../src/examples/tutorial/ascent_intro/notebooks/04_ascent_scene_examples.ipynb>`
* Pipeline Examples: :download:`05_ascent_pipeline_examples.cpp <../../../src/examples/tutorial/ascent_intro/notebooks/05_ascent_pipeline_examples.ipynb>`
* Extract Examples:  :download:`05_ascent_pipeline_examples.cpp <../../../src/examples/tutorial/ascent_intro/notebooks/05_ascent_pipeline_examples.ipynb>`
* Trigger Examples:  :download:`06_ascent_trigger_examples.cpp <../../../src/examples/tutorial/ascent_intro/notebooks/06_ascent_trigger_examples.ipynb>`


C++ Examples
~~~~~~~~~~~~~~~

Example Makefile:
 * :download:`Makefile <../../examples/tutorial/ascent_into/cpp/Makefile>`

First Light:
 * :download:`ascent_example1.cpp <../../../src/examples/tutorial/ascent_intro/cpp/ascent_first_light_example.cpp>`

Conduit Basics:
 * :download:`conduit_example1.cpp <../../../src/examples/tutorial/ascent_intro/cpp/conduit_example1.cpp>`
 * :download:`conduit_example2.cpp <../../../src/examples/tutorial/ascent_intro/cpp/conduit_example2.cpp>`
 * :download:`conduit_example3.cpp <../../../src/examples/tutorial/ascent_intro/cpp/conduit_example3.cpp>`
 * :download:`conduit_example4.cpp <../../../src/examples/tutorial/ascent_intro/cpp/conduit_example4.cpp>`

Conduit Blueprint Mesh Examples:
 * :download:`blueprint_example2.cpp <../../../src/examples/tutorial/ascent_intro/cpp/blueprint_example1.cpp>`
 * :download:`blueprint_example2.cpp <../../../src/examples/tutorial/ascent_intro/cpp/blueprint_example2.cpp>`
 * :download:`blueprint_example2.cpp <../../../src/examples/tutorial/ascent_intro/cpp/blueprint_example3.cpp>`

Scene Examples:
 * :download:`ascent_scene_example1.cpp <../../../src/examples/tutorial/ascent_intro/cpp/ascent_scene_example1.cpp>`
 * :download:`ascent_scene_example2.cpp <../../../src/examples/tutorial/ascent_intro/cpp/ascent_scene_example2.cpp>`
 * :download:`ascent_scene_example3.cpp <../../../src/examples/tutorial/ascent_intro/cpp/ascent_scene_example3.cpp>`
 * :download:`ascent_scene_example4.cpp <../../../src/examples/tutorial/ascent_intro/cpp/ascent_scene_example4.cpp>`


Pipeline Examples:
 * :download:`ascent_pipeline_example1.cpp <../../../src/examples/tutorial/ascent_intro/cpp/ascent_pipeline_example1.cpp>`
 * :download:`ascent_pipeline_example2.cpp <../../../src/examples/tutorial/ascent_intro/cpp/ascent_pipeline_example2.cpp>`
 * :download:`ascent_pipeline_example3.cpp <../../../src/examples/tutorial/ascent_intro/cpp/ascent_pipeline_example3.cpp>`


Extract Examples:
 * :download:`ascent_extract_example1.cpp <../../../src/examples/tutorial/ascent_intro/cpp/ascent_extract_example1.cpp>`
 * :download:`ascent_extract_example2.cpp <../../../src/examples/tutorial/ascent_intro/cpp/ascent_extract_example2.cpp>`
 * :download:`ascent_extract_example2.cpp <../../../src/examples/tutorial/ascent_intro/cpp/ascent_extract_example3.cpp>`

Trigger Examples:
 * :download:`ascent_trigger_example1.cpp <../../../src/examples/tutorial/ascent_intro/cpp/ascent_trigger_example1.cpp>`


Python Examples
~~~~~~~~~~~~~~~

First Light:
 * :download:`ascent_example1.py <../../../src/examples/tutorial/ascent_intro/python/ascent_first_light_example.py>`

Conduit Basics:
 * :download:`conduit_example1.py <../../../src/examples/tutorial/ascent_intro/python/conduit_example1.py>`
 * :download:`conduit_example2.py <../../../src/examples/tutorial/ascent_intro/python/conduit_example2.py>`
 * :download:`conduit_example3.py <../../../src/examples/tutorial/ascent_intro/python/conduit_example3.py>`
 * :download:`conduit_example4.py <../../../src/examples/tutorial/ascent_intro/python/conduit_example4.py>`

Conduit Blueprint Mesh Examples:
 * :download:`blueprint_example2.py <../../../src/examples/tutorial/ascent_intro/python/blueprint_example1.py>`
 * :download:`blueprint_example2.py <../../../src/examples/tutorial/ascent_intro/python/blueprint_example2.py>`
 * :download:`blueprint_example2.py <../../../src/examples/tutorial/ascent_intro/python/blueprint_example3.py>`

Scene Examples:
 * :download:`ascent_scene_example1.py <../../../src/examples/tutorial/ascent_intro/python/ascent_scene_example1.py>`
 * :download:`ascent_scene_example2.py <../../../src/examples/tutorial/ascent_intro/python/ascent_scene_example2.py>`
 * :download:`ascent_scene_example3.py <../../../src/examples/tutorial/ascent_intro/python/ascent_scene_example3.py>`
 * :download:`ascent_scene_example4.py <../../../src/examples/tutorial/ascent_intro/python/ascent_scene_example4.py>`


Pipeline Examples:
 * :download:`ascent_pipeline_example1.py <../../../src/examples/tutorial/ascent_intro/python/ascent_pipeline_example1.py>`
 * :download:`ascent_pipeline_example2.py <../../../src/examples/tutorial/ascent_intro/python/ascent_pipeline_example2.py>`
 * :download:`ascent_pipeline_example3.py <../../../src/examples/tutorial/ascent_intro/python/ascent_pipeline_example3.py>`


Extract Examples:
 * :download:`ascent_extract_example1.py <../../../src/examples/tutorial/ascent_intro/python/ascent_extract_example1.py>`
 * :download:`ascent_extract_example2.py <../../../src/examples/tutorial/ascent_intro/python/ascent_extract_example2.py>`
 * :download:`ascent_extract_example2.py <../../../src/examples/tutorial/ascent_intro/python/ascent_extract_example3.py>`

Trigger Examples:
 * :download:`ascent_trigger_example1.py <../../../src/examples/tutorial/ascent_intro/python/ascent_trigger_example1.py>`


.. ECP 2019 Annual Meeting
.. -------------------------
..
.. **In Situ Visualization and Analysis with Ascent**
..
.. **Date:** Thursday, January 17th, 2019
..
.. **Time:** 8:30am - 12:00pm
..
.. **Location:** Champions VII
..
.. **Authors:**
..
.. Hank Childs (University of Oregon); Matthew Larsen (Lawrence Livermore National Laboratory); Cyrus Harrison (Lawrence Livermore National Laboratory); Kenneth Moreland (Sandia National Laboratories); David Rogers (Los Alamos National Laboratory)
..
.. **Abstract:**
..
.. In situ visualization and analysis is an important capability for addressing slow I/O on modern supercomputers. With this 3-hour tutorial, we will spend the majority of our time (two hours) going into detail on Ascent, an in situ visualization and analysis library being developed by ECP ALPINE. Ascent is from the makers of ParaView Catalyst and VisIt LibSim, and it will soon be able to directly connect with both of those products. The tutorial will be practical in nature: how to integrate Ascent into a simulation code, Ascent’s data model, build and linking issues, and capabilities. The remaining hour will be spent highlighting other visualization efforts in ECP, such as in situ-specific visualization algorithms, VTK-m, and CINEMA.
..
.. **Slides:**
.. `http://portal.nersc.gov/project/alpine/docs/ecp_2019_ahm/  <http://portal.nersc.gov/project/alpine/docs/ecp_2019_ahm/>`_
