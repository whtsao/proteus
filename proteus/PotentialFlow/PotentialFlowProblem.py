from __future__ import division
from past.utils import old_div
from proteus import FemTools as ft
from proteus import MeshTools as mt
import proteus.default_p as physics 
import proteus.default_n as numerics
from proteus.TransportCoefficients import PoissonEquationCoefficients

class Hi:
    def __init__(self): 
        pass
    def hello(self):
        return 'Hello from PotentialFlowProblem.py'


class PotentialFlowProblem:
    """ PotentialFlowProblem """

    def __init__(self,
                 time_stepping_scheme='RK4',  # 'RK4' or 'tse'
                 cfl=0.5,
                 # DOMAIN AND MESH #
                 structured=False,
                 he=None,
                 nnx=None,
                 nny=None,
                 domain=None,
                 mesh=None,
                 # INITIAL CONDITIONS #
                 initialConditions=None,
                 # BOUNDARY CONDITIONS #
                 boundaryConditions=None,
                 # FREE SURFACE PARAMETERS #
                 freeSurface=None,
                 gravity=9.81,
                 auxiliaryVariables=None,
                 genMesh=True):

        # ***** ASSERTIONS ***** #
        assert cfl <= 1, "Choose cfl <= 1"
        assert domain.nd in [2], "use nd = 2 at the moment"
        assert domain is not None, "Provide a domain"
        assert time_stepping_scheme in ['RK4', 'tse'], "time_stepping_scheme must be 'RK4' or 'tse'"
        assert isinstance(outputStepping, OutputStepping), "Provide an object from the OutputStepping class"
        assert isinstance(he, float), "Provide (float) he (characteristic mesh size)"
        if structured:
            assert isinstance(nnx, int) and isinstance(nny, int), "Provide (int) nnx and (int) nny"
        assert callable(freeSurface), "freeSurface must be a function defining the initial surface profile"
        assert isinstance(initialConditions, dict), "Provide dict of initial conditions"
        assert isinstance(boundaryConditions, dict), "Provide dict of boundary conditions"
        # self.assert_initialConditions(initialConditions)
        # self.assert_boundaryConditions(boundaryConditions)

        # ***** SAVE PARAMETERS ***** #
        self.genMesh = genMesh
        self.cfl = cfl
        self.time_stepping_scheme = time_stepping_scheme
        self.outputStepping = outputStepping.getOutputStepping()
        self.he = he
        self.nnx = nnx
        self.nny = nny
        self.nnz = 1
        self.domain = domain
        self.mesh = mesh
        self.initialConditions = initialConditions
        self.boundaryConditions = boundaryConditions
        self.freeSurface = freeSurface
        self.gravity = gravity
        self.auxiliaryVariables = auxiliaryVariables or []

        # ***** CREATE FINITE ELEMENT SPACES ***** #
        self.FESpace = FESpace().getFESpace()

        # ***** DEFINE PHYSICAL AND NUMERICAL PARAMETERS ***** #
        self.physical_parameters = default_physical_parameters
        self.potential_flow_parameters = default_potential_flow_parameters

    def assert_initialConditions(self, NinitialConditions):
        assert 'potential' in initialConditions, 'Provide initial potential in initialConditions'
        assert 'velocity_x' in initialConditions, 'Provide x-component of initial velocity in initialConditions'
        assert 'velocity_y' in initialConditions, 'Provide y-component of initial velocity in initialConditions'

    def assert_boundaryConditions(self, boundaryConditions):
        assert 'potential' in boundaryConditions, 'Provide potential in boundaryConditions'
        assert 'velocity_x' in boundaryConditions, 'Provide x-component of velocity in boundaryConditions'
        assert 'velocity_y' in boundaryConditions, 'Provide y-component of velocity in boundaryConditions'

        
class OutputStepping:
    """
    OutputStepping handles how often the solution is outputted.
    """
    def __init__(self,
                 final_time,
                 dt_init=0.001,
                 dt_output=None,
                 nDTout=None):
        self.final_time = final_time
        self.dt_init = dt_init
        assert not (dt_output is None and nDTout is None), "Provide dt_output or nDTout"
        self.dt_output = dt_output
        self.nDTout = nDTout

    def getOutputStepping(self):
        dt_init = min(0.1 * self.dt_output, self.dt_init)
        if self.nDTout is None:
            self.nDTout = int(round(self.final_time/self.dt_output))
        else:
            self.dt_output = float(self.final_time) / float(self.nDTout)
        return {'final_time': self.final_time,
                'dt_init': dt_init,
                'dt_output': self.dt_output,
                'nDTout': self.nDTout}
class FESpace:
    """
    Create FE Spaces.
    """

    def __init__(self):
        pass

def getFESpace(self):
    nd = self.domain.nd  # or define as needed (e.g., 2 for 2D problems)
    assert nd in [2, 3], "Number of dimensions must be 2 or 3"

    # Choose the basis for potential and velocity
    # For potential, we use P1 (linear basis); if higher-order accuracy is required, adjust accordingly
    self.potentialBasis = ft.C0_AffineLinearOnSimplexWithNodalBasis  # P1 space for potential
    self.hFactor = 1.0  # Mesh size factor, adjust if needed

    # Quadrature Rule Setup
    # Define element quadrature and boundary quadrature based on problem needs
    useExact = self.SystemNumerics.useExact if hasattr(self, 'SystemNumerics') else False
    if useExact:
        quadOrder = 6
    else:
        quadOrder = 3 if self.potentialBasis == ft.C0_AffineLinearOnSimplexWithNodalBasis else 5

    # Select quadrature rules for elements and boundaries based on chosen basis functions
    self.elementQuadrature = ft.SimplexGaussQuadrature(nd, quadOrder)
    self.elementBoundaryQuadrature = ft.SimplexGaussQuadrature(nd - 1, quadOrder)

    return {
        'potentialBasis': self.potentialBasis,  # Basis for the potential field
        'elementQuadrature': self.elementQuadrature,
        'elementBoundaryQuadrature': self.elementBoundaryQuadrature
    }


# ***************************************** #
# ********** PHYSICAL PARAMETERS ********** #
# ***************************************** #
default_physical_parameters = {'gravity': 9.81}

# ****************************************** #
# ********** NUMERICAL PARAMETERS ********** #
# ****************************************** #
default_potential_flow_parameters = {'LUMPED_MASS_MATRIX': 0,
                                     'cfl': 0.5,
                                     'RKOrder': 4,  # Runge-Kutta order
                                     'explicit_scheme': True}


        
        # # ***** CREATE SYSTEM PHYSICS OBJECT ***** #
        # def r(x):
        #     return 0.
        # def a(x):
        #     if self.domain.nd == 2:
        #         return np.array([[1.,0.],
        #                          [0.,1.]])
        #     elif self.domain.nd == 3:
        #         return np.array([[1.,0.,0.],
        #                          [0.,1.,0.],
        #                          [0.,0.,1.]])
        # physics.coefficients = PoissonEquationCoefficients([a],[r]) 
        # physics.name="PoissonDots"

        #  # ***** CREATE SYSTEM PHYSICS OBJECT ***** #
        # self.SystemPhysics = SystemPhysics(ProblemInstance=self)
        # self.SystemPhysics.initialConditions = initialConditions
        # self.SystemPhysics.boundaryConditions= boundaryConditions        
        
        # # ***** CREATE SYSTEM NUMERICS OBJECT ***** #
        # self.SystemNumerics = SystemNumerics(ProblemInstance=self)

        # # ***** CREATE MODEL PARAMETERS OBJECT ***** #
        # self.Parameters = Parameters.ParametersHolder(ProblemInstance=self)

        # # ***** CREATE FINITE ELEMENT SPACES ***** #
        # self.FESpace = FESpace(ProblemInstance=self)
        # self.FESpace = FESpace().getFESpace()
        
        # # ***** CREATING OUTPUT MANAGEMENT OBJECTS ***** #
        # self.so = System_base()
        # self.outputStepping=OutputStepping()

        


        # # ***** DEFINE PHYSICAL AND NUMERICAL PARAMETERS ***** #
        # self.physical_parameters = default_physical_parameters
        # self.swe_parameters = default_swe_parameters
        # self.GN_swe_parameters = default_GN_swe_parameters

if __name__ == "__main__":
    instance = Hi()
    print(instance.hello())
