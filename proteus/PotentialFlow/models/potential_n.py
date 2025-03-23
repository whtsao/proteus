#!/usr/bin/env python
# coding: utf-8

# In[ ]:


from proteus import *
from proteus.default_n import *
from potential_p import *
from proteus.Transport import Comm

# *********************************************** #
# ********** Read from myPotentialFlowProblem *** #
# *********************************************** #
# READ FROM CONTEXT #
runCFL = myPotentialFlowProblem.cfl
FESpace = myPotentialFlowProblem.FESpace
he = myPotentialFlowProblem.he
useSuperlu = myPotentialFlowProblem.useSuperlu
domain = myPotentialFlowProblem.domain
auxiliaryVariables = myPotentialFlowProblem.auxiliaryVariables
time_stepping_scheme = myPotentialFlowProblem.time_stepping_scheme

# *************************************** #
# ********** MESH CONSTRUCTION ********** #
# *************************************** #
if domain is not None:
    triangleFlag = myPotentialFlowProblem.triangleFlag
    nnx = myPotentialFlowProblem.nnx
    nny = myPotentialFlowProblem.nny
    nnz = myPotentialFlowProblem.nnz
    triangleOptions = domain.MeshOptions.triangleOptions

# ************************************** #
# ********** TIME INTEGRATION ********** #
# ************************************** #
# Choose time integration method based on time-stepping scheme
if time_stepping_scheme == 'RK2':
    timeIntegration = TimeIntegration.RKEV  # Runge-Kutta method
    timeOrder = 2
    nStagesTime = 2
else:
    timeIntegration = TimeIntegration.ForwardEuler  # explicit scheme
    timeOrder = 1
    nStagesTime = 1

# ****************************************** #
# ********** TIME STEP CONTROLLER ********** #
# ****************************************** #
stepController = Min_dt_controller

# ******************************************* #
# ********** FINITE ELEMENT SPACES ********** #
# ******************************************* #
elementQuadrature = FESpace['elementQuadrature']
elementBoundaryQuadrature = FESpace['elementBoundaryQuadrature']
femSpaces = {0: FESpace['potentialBasis']}  # Only one variable (potential)

# ************************************** #
# ********** NONLINEAR SOLVER ********** #
# ************************************** #
multilevelNonlinearSolver = Newton
fullNewtonFlag = False  # For explicit schemes, Newton can be set to False
levelNonlinearSolver = ExplicitLumpedMassMatrixPotentialFlowSolver

# ************************************ #
# ********** NUMERICAL FLUX ********** #
# ************************************ #
numericalFluxType = NumericalFlux.PotentialFlow_NumericalFlux

# ************************************ #
# ********** LINEAR ALGEBRA ********** #
# ************************************ #
matrix = SparseMatrix
multilevelLinearSolver = LU
levelLinearSolver = LU
# Adjust solver for parallel runs
comm = Comm.get()
if comm.size() > 1:
    levelLinearSolver = KSP_petsc4py
    multilevelLinearSolver = KSP_petsc4py
levelNonlinearSolverConvergenceTest = 'r'
linearSolverConvergenceTest = 'r-true'

# ******************************** #
# ********** TOLERANCES ********** #
# ******************************** #
nl_atol_res = 1.0e-7
nl_rtol_res = 0.0
l_atol_res = 1.0e-7
l_rtol_res = 0.0
tolFac = 0.0
maxLineSearches = 0

