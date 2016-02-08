.. quickstart examples


Quick start examples
====================

.. contents::


C++
---

After following the :ref:`installationguide` you will be able to compile and run your first C++ AuDi program:

.. _getting_started:

.. literalinclude:: ../../doc/examples/getting_started.cpp
   :language: c++
   :linenos:

Place it into a getting_started.cpp text file and compile it with:

.. code-block:: bash

   g++ -std=c++11 getting_started.cpp -lmpfr -lgmp -pthread

-----------------------------------------------------------------------

Python
------

If you have succesfully compiled and installed pyaudi following the :ref:`installationguide` you will be able to test its use typing the following script.

.. literalinclude:: ../../doc/examples/getting_started.py
   :language: python
   :linenos:

Place it into a getting_started.py text file and run it with 

.. code-block:: bash

   python getting_started.py

We reccomend the use of Jupyter or ipython do enjoy pyaudi the most.