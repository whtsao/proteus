from pyadh import *
from pyadh.default_n import *
from ls_sloshbox_3d_old_p import *

if useBackwardEuler_ls:
    timeIntegration = BackwardEuler_cfl
    stepController = Min_dt_controller
#     timeIntegration = BackwardEuler
#     stepController = FixedStep
else:
    timeIntegration = FLCBDF
    stepController = FLCBDF_controller_sys
    rtol_u[0] = 1.0e-2
    atol_u[0] = 1.0e-2

if spaceOrder == 1:
    femSpaces = {0:C0_AffineLinearOnSimplexWithNodalBasis}
elif spaceOrder == 2:
    femSpaces = {0:C0_AffineQuadraticOnSimplexWithNodalBasis}

elementQuadrature = SimplexGaussQuadrature(nd,sloshbox_quad_order)

elementBoundaryQuadrature = SimplexGaussQuadrature(nd-1,sloshbox_quad_order)


subgridError = HamiltonJacobi_ASGS(coefficients,nd,lag=False)#it's  linear anyway

massLumping = False

numericalFluxType = None

shockCapturing = None
#shockCapturing = ResGrad_SC(coefficients,nd,shockCapturingFactor=ls_shockCapturingFactor,lag=lag_ls_shockCapturing)
shockCapturing = ResGradQuad_SC(coefficients,nd,shockCapturingFactor=ls_shockCapturingFactor,lag=lag_ls_shockCapturing)

multilevelNonlinearSolver  = Newton#NLNI

levelNonlinearSolver = Newton

nonlinearSmoother = NLGaussSeidel

fullNewtonFlag = True

tolFac = 0.0

nl_atol_res = 0.01*he#1.0e-4#should be linear with lagging

maxNonlinearIts = 50

matrix = SparseMatrix

if usePETSc:
    numericalFluxType = DoNothing

    multilevelLinearSolver = PETSc
    
    levelLinearSolver = PETSc
else:
    numericalFluxType = DoNothing
    
    multilevelLinearSolver = LU
    
    levelLinearSolver = LU
   
linearSmoother = GaussSeidel

linTolFac = 0.001

conservativeFlux = None
