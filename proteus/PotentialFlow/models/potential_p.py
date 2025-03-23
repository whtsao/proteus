#!/usr/bin/env python
# coding: utf-8

# In[ ]:


from proteus import *
from proteus.default_p import *
from proteus.Domain import RectangularDomain
import numpy as np
from proteus import Context

# *********************************************** #
# ********** READ FROM myPotentialFlowProblem *** #
# *********************************************** #
ct = Context.get()
myPotentialFlowProblem = ct.myPotentialFlowProblem
genMesh = myPotentialFlowProblem.genMesh
physical_parameters = myPotentialFlowProblem.physical_parameters
initialConditions = myPotentialFlowProblem.initialConditions
boundaryConditions = myPotentialFlowProblem.boundaryConditions
freeSurface = myPotentialFlowProblem.freeSurface
analyticalSolution = myPotentialFlowProblem.analyticalSolution

# DOMAIN #
nd = 2  # Dimension (2D problem)
domain = myPotentialFlowProblem.domain
if domain is None:
    meshfile = myPotentialFlowProblem.AdH_file

# ******************************** #
# ********** PARAMETERS ********** #
# ******************************** #

# PHYSICAL PARAMETERS #
g = physical_parameters['gravity']

# ********************************** #
# ********** COEFFICIENTS ********** #
# ********************************** #
# Coefficients for potential flow model (Laplace equation)
class PotentialFlowCoefficients:
    def __init__(self, gravity, freeSurface=None):
        self.gravity = gravity
        self.freeSurface = freeSurface

coefficients = PotentialFlowCoefficients(g=g, freeSurface=freeSurface)

# **************************************** #
# ********** INITIAL CONDITIONS ********** #
# **************************************** #
# Define the initial potential and velocity field
initialConditions = {0: initialConditions['potential'],
                     1: initialConditions['velocity_x'],
                     2: initialConditions['velocity_y']}

# ***************************************** #
# ********** BOUNDARY CONDITIONS ********** #
# ***************************************** #
# Set Dirichlet boundary conditions for potential and Neumann for velocity
dirichletConditions = {0: boundaryConditions['potential'],
                       1: boundaryConditions['velocity_x'],
                       2: boundaryConditions['velocity_y']}
fluxBoundaryConditions = {0: 'outFlow',
                          1: 'outFlow',
                          2: 'outFlow'}
advectiveFluxBoundaryConditions = {0: lambda x, flag: None,
                                   1: lambda x, flag: None,
                                   2: lambda x, flag: None}
diffusiveFluxBoundaryConditions = {0: {},
                                   1: {1: lambda x, flag: None},
                                   2: {2: lambda x, flag: None}}

# **************************************** #
# ********** ANALYTICAL SOLUTION ********* #
# **************************************** #
# Define analytical solution if available
if myPotentialFlowProblem.analyticalSolution is not None:
    analyticalSolution = {0: analyticalSolution['potential_exact'],
                          1: analyticalSolution['velocity_x_exact'],
                          2: analyticalSolution['velocity_y_exact']}

