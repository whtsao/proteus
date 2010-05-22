#include "RDLS2DQ.h"
#include <iostream>
#include <cassert>

//for now assumes that using time integration
//and so lags stabilization and subgrid error
//extern "C" void calculateResidual_RDLSQ(int nElements_global,
extern "C" void RDLSQ_RES (int nElements_global,
			   double alphaBDF,
			   double epsFact_redist,
			   int freezeLevelSet,
			   int useTimeIntegration,
			   int lag_shockCapturing, 
			   int lag_subgridError, //0 nothing lagged
				                 //1 dH lagged in tau
                                                 //2 dH lagged in tau and Residual, adjoint calculations
			   double shockCapturingDiffusion,
			   int* u_l2g, 
			   double* elementDiameter,
			   double* u_dof,
			   double* u_trial, 
			   double* u_grad_trial, 
			   double* u_test_dV, 
			   double* u_grad_test_dV, 
			   double* phi_ls,
			   double* q_m,
			   double* q_u,
			   double* q_dH,
			   double* u_weak_internal_bc_dofs,//for freezing level set
			   double* q_m_betaBDF,
			   double* q_dH_last,//for lagging subgrid error
			   double* q_cfl,
			   double* q_numDiff_u, 
			   double* q_numDiff_u_last, 
			   double* q_elementResidual_u,
			   //mwf for debugging
			   double* q_m_t,
			   double* q_r,
			   double* q_subgridError,
			   double* q_Lstar,
			   double* q_tau_last,
			   //mwf end debugging
			   int* weakDirichletConditionFlags,
			   int offset_u, int stride_u, 
			   double* globalResidual,
			   int nExteriorElementBoundaries_global,
			   int* exteriorElementBoundariesArray,
			   int* elementBoundaryElementsArray,
			   int* elementBoundaryLocalElementBoundariesArray,
			   double* u_trial_ext,
			   double* u_grad_trial_ext,
			   double* ebqe_phi_ls_ext,
			   double* ebqe_n_ext,
			   int* isDOFBoundary_u,
			   double* ebqe_bc_u_ext,
			   double* u_test_dS_ext,
			   double* ebqe_u)
{
  //lag_shockCapturing=1;
  //lag_subgridError=2;
  //using namespace RDLSQ;
  using namespace RDLSQ_NAME; 
  //
  //loop over elements to compute volume integrals and load them into element and global residual
  //
  //eN is the element index
  //eN_k is the quadrature point index for a scalar
  //eN_k_nSpace is the quadrature point index for a vector
  //eN_i is the element test function index
  //eN_j is the element trial function index
  //eN_k_j is the quadrature point index for a trial function
  //eN_k_i is the quadrature point index for a trial function
  double timeIntegrationScale = 1.0;
  if (useTimeIntegration == 0)
    timeIntegrationScale = 0.0;
  double lag_shockCapturingScale = 1.0;
  if (lag_shockCapturing == 0)
    lag_shockCapturingScale = 0.0;
  for(int eN=0;eN<nElements_global;eN++)
    {
      //declare local storage for element residual and initialize
      register double elementResidual_u[nDOF_test_element];
      const double epsilon_redist= epsFact_redist*elementDiameter[eN];
      for (int i=0;i<nDOF_test_element;i++)
	{
	  elementResidual_u[i]=0.0;
	}//i
      //loop over quadrature points and compute integrands
      for  (int k=0;k<nQuadraturePoints_element;k++)
        {
	  //compute indeces and declare local storage
	  register int eN_k = eN*nQuadraturePoints_element+k,
	    eN_k_nSpace = eN_k*nSpace;
	  register double u=0.0,grad_u[nSpace],
	    m=0.0,dm=0.0,
	    H=0.0,dH[nSpace],
	    m_t=0.0,dm_t=0.0,
	    r=0.0,
	    dH_tau[nSpace],//dH if not lagging or q_dH_last if lagging tau
	    dH_strong[nSpace],//dH if not lagging or q_dH_last if lagging strong residual and adjoint
	    pdeResidual_u=0.0,
	    Lstar_u[nDOF_test_element],
	    subgridError_u=0.0,
	    tau=0.0,
	    nu_sc=0.0;
          //
          //compute solution and gradients at quadrature points
          //
	  u=0.0;
	  for (int I=0;I<nSpace;I++)
	    {
	      grad_u[I]=0.0;
	    }
          for (int j=0;j<nDOF_trial_element;j++)
            {
	      int eN_j=eN*nDOF_trial_element+j;
	      int eN_k_j=eN_k*nDOF_trial_element+j;
	      int eN_k_j_nSpace = eN_k_j*nSpace;
	      u += valFromDOF_c(u_dof[u_l2g[eN_j]],u_trial[eN_k_j]);
	      for (int I=0;I<nSpace;I++)
		{
		  grad_u[I] += gradFromDOF_c(u_dof[u_l2g[eN_j]],u_grad_trial[eN_k_j_nSpace+I]);
		}
	    }
          //
          //calculate pde coefficients at quadrature points
          //
          evaluateCoefficients_c(epsilon_redist,
				 phi_ls[eN_k],
				 u,
				 grad_u,
				 m,
				 dm,
				 H,
				 dH,
				 r);
	  //TODO allow not lagging of subgrid error etc,
	  //remove conditional?
	  //default no lagging
	  for (int I=0; I < nSpace; I++)
	    {
	      dH_tau[I] = dH[I];
	      dH_strong[I] = dH[I];
	    }
	  if (lag_subgridError > 0)
	    {
	      for (int I=0; I < nSpace; I++)
		{
		  dH_tau[I] = q_dH_last[eN_k_nSpace+I];
		}
	    }
	  if (lag_subgridError > 1)
	    {
	      for (int I=0; I < nSpace; I++)
		{
		  dH_strong[I] = q_dH_last[eN_k_nSpace+I];
		}
	    }
// 	  if (lag_subgridError)
// 	    {
// 	      for (int I=0; I < nSpace; I++)
// 		{
// 		  dH_sge[I] = q_dH_last[eN_k_nSpace+I];
// 		}
// 	    }
// 	  else
// 	    {
// 	      for (int I=0; I < nSpace; I++)
// 		{
// 		  dH_sge[I] = dH[I];
// 		}
// 	    }
	  //save mass for time history and dH for subgrid error
	  //save solution for other models 
	  //
	  q_m[eN_k] = m;
	  q_u[eN_k] = u;
	  for (int I=0;I<nSpace;I++)
	    {
	      int eN_k_nSpace_I = eN_k_nSpace+I;
	      q_dH[eN_k_nSpace_I] = dH[I];                     
	    }

          //
          //moving mesh
          //
          //omit for now
          //
          //calculate time derivative at quadrature points
          //

          bdf_c(alphaBDF,
		q_m_betaBDF[eN_k],
		m,
		dm,
		m_t,
		dm_t);
	  //TODO add option to skip if not doing time integration (Newton stead-state solve)
	  m *= timeIntegrationScale; dm *= timeIntegrationScale; m_t *= timeIntegrationScale;
	  dm_t *= timeIntegrationScale;
          //
          //calculate subgrid error (strong residual and adjoint)
          //
          //calculate strong residual
	  pdeResidual_u = Mass_strong_c(m_t) +
	    Hamiltonian_strong_c(dH_strong,grad_u) + //would need dH if not lagging
	    Reaction_strong_c(r);

          //calculate adjoint
          for (int i=0;i<nDOF_test_element;i++)
            {
	      register int eN_k_i_nSpace = (eN_k*nDOF_trial_element+i)*nSpace;
	      Lstar_u[i]  = Hamiltonian_adjoint_c(dH_strong,&u_grad_test_dV[eN_k_i_nSpace]);
	      //reaction is constant
            }
          //calculate tau and tau*Res
          calculateSubgridError_tau_c(elementDiameter[eN],
				      dm_t,dH_tau,
				      q_cfl[eN_k],
				      tau);
          calculateSubgridError_tauRes_c(tau,
					 pdeResidual_u,
					 subgridError_u);
          //
          //calcualte shock capturing diffusion
          //
          calculateNumericalDiffusion_c(shockCapturingDiffusion,elementDiameter[eN],pdeResidual_u,grad_u,q_numDiff_u[eN_k]);

	  nu_sc = q_numDiff_u[eN_k]*(1.0-lag_shockCapturingScale) + q_numDiff_u_last[eN_k]*lag_shockCapturingScale;
          // 
          //update element residual 
          // 
          for(int i=0;i<nDOF_test_element;i++) 
	    { 
	      register int eN_k_i=eN_k*nDOF_test_element+i,
		eN_k_i_nSpace = eN_k_i*nSpace;
	       

	      elementResidual_u[i] += Mass_weak_c(m_t,u_test_dV[eN_k_i]) + 
		Hamiltonian_weak_c(H,u_test_dV[eN_k_i]) + 
		Reaction_weak_c(r,u_test_dV[eN_k_i]) +
		SubgridError_c(subgridError_u,Lstar_u[i]) + 
		NumericalDiffusion_c(nu_sc,grad_u,&u_grad_test_dV[eN_k_i_nSpace]); 

            }//i
	  //


// 	  //mwf debug
// 	  if (true || dH[0]*dH_sge[0] <= 0.0)
// 	    {
// 	      std::cout<<"RDLS2DQ res eN= "<<eN<<" u= "<<u<<" m= "<<m<<" alphaBDF= "<<alphaBDF<<" m_t= "<<m_t<<" dm_t= "<<dm_t<<" r= "<<r<<" dH[0]= "<< dH[0]<<" dH_sge[0]= "<< dH_sge[0]<<" grad_u[0]="<<grad_u[0]
// 		       <<"\n\t tau= "<<tau<<" pdeResidual= "<<pdeResidual_u
// 		       <<" numDiff= "<<q_numDiff_u[eN_k]<<" numDiff_last= "<<q_numDiff_u_last[eN_k]<<std::endl;
// 	    }
	  //mwf for debugging
	  q_m_t[eN_k] = m_t;
	  q_r[eN_k]   = r;
	  q_subgridError[eN_k] = subgridError_u;
	  q_tau_last[eN_k]   = tau;
          for(int i=0;i<nDOF_test_element;i++) 
	    { 
	      register int eN_k_i=eN_k*nDOF_test_element+i;
	      q_Lstar[eN_k_i] = Lstar_u[i];
	    }//i
	}//k

      //
      //apply weak constraints for unknowns near zero level set
      //
      //
      if (freezeLevelSet)
	{
	  const double weakDirichletFactor = 3.0;
	  for (int j = 0; j < nDOF_trial_element; j++)
	    {
	      const int eN_j = eN*nDOF_trial_element+j;
	      const int J = u_l2g[eN_j];
	      if (weakDirichletConditionFlags[J] == 1)
		{
		  elementResidual_u[j] = (u_dof[J]-u_weak_internal_bc_dofs[J])*weakDirichletFactor*elementDiameter[eN];
		  //mwf debug
		  //std::cout<<"RDLSQ freeze eN= "<<eN<<" j= "<<j<<" J= "<<J<<" u_dof= "<<u_dof[J]<<" u_weak_bc= "<<u_weak_internal_bc_dofs[J]<<std::endl;
		}
	    }//j
	}//freeze

      //
      //load element into global residual and save element residual
      //
      for(int i=0;i<nDOF_test_element;i++) 
        { 
          register int eN_i=eN*nDOF_test_element+i;
          
          q_elementResidual_u[eN_i]+=elementResidual_u[i];

          globalResidual[offset_u+stride_u*u_l2g[eN_i]]+=elementResidual_u[i];
        }//i
    }//elements
  //
  //loop over exterior element boundaries to calculate surface integrals and load into element and global residuals
  //
  //ebNE is the Exterior element boundary INdex
  //ebN is the element boundary INdex
  //eN is the element index
  for (int ebNE = 0; ebNE < nExteriorElementBoundaries_global; ebNE++) 
    { 
      register int ebN = exteriorElementBoundariesArray[ebNE], 
	eN  = elementBoundaryElementsArray[ebN*2+0];
      register double elementResidual_u[nDOF_test_element];
      const double epsilon_redist= epsFact_redist*elementDiameter[eN];
      for (int i=0;i<nDOF_test_element;i++)
	{
	  elementResidual_u[i]=0.0;
	}
      for  (int kb=0;kb<nQuadraturePoints_elementBoundary;kb++) 
	{ 
	  register int ebNE_kb = ebNE*nQuadraturePoints_elementBoundary+kb,
	    ebNE_kb_nSpace = ebNE_kb*nSpace;
	  register double u_ext=0.0,
	    grad_u_ext[nSpace],
	    m_ext=0.0,
	    dm_ext=0.0,
	    H_ext=0.0,
	    dH_ext[nSpace],
	    r_ext=0.0,
	    flux_ext=0.0,
	    bc_u_ext=0.0,
	    bc_grad_u_ext[nSpace],
	    bc_m_ext=0.0,
	    bc_dm_ext=0.0,
	    bc_H_ext=0.0,
	    bc_dH_ext[nSpace],
	    bc_r_ext=0.0;
	  // 
	  //calculate the solution and gradients at quadrature points 
	  // 
	  u_ext=0.0;
	  for (int I=0;I<nSpace;I++)
	    {
	      grad_u_ext[I] = 0.0;
	      bc_grad_u_ext[I] = 0.0;/*mwf need way to evaluate this*/
	    }
	  for (int j=0;j<nDOF_trial_element;j++) 
	    { 
	      int eN_j = eN*nDOF_trial_element+j;
	      int ebNE_kb_j = ebNE_kb*nDOF_trial_element+j;
	      int ebNE_kb_j_nSpace= ebNE_kb_j*nSpace;
	      u_ext += valFromDOF_c(u_dof[u_l2g[eN_j]],u_trial_ext[ebNE_kb_j]); 
	      for (int I=0;I<nSpace;I++)
		{
		  grad_u_ext[I] += gradFromDOF_c(u_dof[u_l2g[eN_j]],u_grad_trial_ext[ebNE_kb_j_nSpace+I]); 
		} 
	    }
	  //
	  //load the boundary values
	  //
	  bc_u_ext = isDOFBoundary_u[ebNE_kb]*ebqe_bc_u_ext[ebNE_kb]+(1-isDOFBoundary_u[ebNE_kb])*u_ext;

	  // 
	  //calculate the pde coefficients using the solution and the boundary values for the solution 
	  // 
	  evaluateCoefficients_c(epsilon_redist,
				 ebqe_phi_ls_ext[ebNE_kb],
				 u_ext,
				 grad_u_ext,
				 m_ext,
				 dm_ext,
				 H_ext,
				 dH_ext,
				 r_ext);
	  evaluateCoefficients_c(epsilon_redist,
				 ebqe_phi_ls_ext[ebNE_kb],
				 bc_u_ext,
				 bc_grad_u_ext,
				 bc_m_ext,
				 bc_dm_ext,
				 bc_H_ext,
				 bc_dH_ext,
				 bc_r_ext);
	  //save for other models?
	  ebqe_u[ebNE_kb] = u_ext;
	  // 
	  //calculate the numerical fluxes 
	  // 
	  //DoNothing for now
	  //
	  //update residuals
	  //
// 	  if (kb >= 0)
// 	    {
// 	      std::cout<<"RDLSQ res ebNE= "<<ebNE<<" ebN= "<<ebN<<" kb= "<<kb<<" ebNE_kb= "<<ebNE_kb <<" u_ext= "<<u_ext<<" m_ext= "<<m_ext
// 		       <<std::endl;
// 	    }
	  for (int i=0;i<nDOF_test_element;i++)
	    {
	      int ebNE_kb_i = ebNE_kb*nDOF_test_element+i;
	      //mwf debug
	      assert(flux_ext == 0.0);
	      elementResidual_u[i] += ExteriorElementBoundaryFlux_c(flux_ext,u_test_dS_ext[ebNE_kb_i]);
	    }//i
	}//kb
      //
      //update the element and global residual storage
      //
      for (int i=0;i<nDOF_test_element;i++)
	{
	  int eN_i = eN*nDOF_test_element+i;
	  q_elementResidual_u[eN_i]+=elementResidual_u[i];

	  globalResidual[offset_u+stride_u*u_l2g[eN_i]]+=elementResidual_u[i];
	}//i
    }//ebNE

}

//for now assumes that using time integration
//and so lags stabilization and subgrid error
//extern "C" void calculateJacobian_RDLSQ(int nElements_global,
extern "C" void RDLSQ_JAC (int nElements_global,
			   double alphaBDF,
			   double epsFact_redist,
			   int freezeLevelSet,
			   int useTimeIntegration,
			   int lag_shockCapturing,
			   int lag_subgridError, 
			   double shockCapturingDiffusion,
			   int* u_l2g,
			   double* elementDiameter,
			   double* u_dof, 
			   double* u_trial, 
			   double* u_grad_trial, 
			   double* u_test_dV, 
			   double* u_grad_test_dV, 
			   double* phi_ls,
			   double* q_m_betaBDF,
			   double* q_dH_last,
			   double* q_cfl,
			   double* q_numDiff_u,
			   double* q_numDiff_u_last,
			   int * weakDirichletConditionFlags,
			   int* csrRowIndeces_u_u,int* csrColumnOffsets_u_u,
			   double* globalJacobian,
			   int nExteriorElementBoundaries_global,
			   int* exteriorElementBoundariesArray,
			   int* elementBoundaryElementsArray,
			   int* elementBoundaryLocalElementBoundariesArray,
			   double* u_trial_ext,
			   double* u_grad_trial_ext,
			   double* ebqe_phi_ls_ext,
			   double* ebqe_n,
			   int* isDOFBoundary_u,
			   double* ebqe_bc_u_ext,
			   double* u_test_dS_ext,
			   int* csrColumnOffsets_eb_u_u)
{
  //lag_shockCapturing=1;
  //lag_subgridError=2;
  //using namespace RDLSQ;
  using namespace RDLSQ_NAME;
  //
  //loop over elements to compute volume integrals and load them into the element Jacobians and global Jacobian
  //
  double timeIntegrationScale = 1.0;
  if (useTimeIntegration == 0)
    timeIntegrationScale = 0.0;
  double lag_shockCapturingScale = 1.0;
  if (lag_shockCapturing == 0)
    lag_shockCapturingScale = 0.0;
  for(int eN=0;eN<nElements_global;eN++)
    {
      register double  elementJacobian_u_u[nDOF_test_element][nDOF_trial_element];
      const double epsilon_redist=epsFact_redist*elementDiameter[eN];
      for (int i=0;i<nDOF_test_element;i++)
	for (int j=0;j<nDOF_trial_element;j++)
	  {
	    elementJacobian_u_u[i][j]=0.0;
	  }
      for  (int k=0;k<nQuadraturePoints_element;k++)
        {
	  int eN_k = eN*nQuadraturePoints_element+k, //index to a scalar at a quadrature point
	    eN_k_nSpace = eN_k*nSpace; //index to a vector at a quadrature point

	  //declare local storage
	  register double u=0.0,
	    grad_u[nSpace],
	    m=0.0,dm=0.0,
	    H=0.0,dH[nSpace],
	    m_t=0.0,dm_t=0.0,r=0.0,
	    dH_tau[nSpace],//dH or dH_last if lagging for tau formula
	    dH_strong[nSpace],//dH or dH_last if lagging for strong residual and adjoint
	    dpdeResidual_u_u[nDOF_trial_element],
	    Lstar_u[nDOF_test_element],
	    dsubgridError_u_u[nDOF_trial_element],
	    tau=0.0,
	    nu_sc=0.0;
          //
          //calculate solution and gradients at quadrature points
          //
	  u=0.0;
	  for (int I=0;I<nSpace;I++)
	    {
	      grad_u[I]=0.0;
	    }
          for (int j=0;j<nDOF_trial_element;j++)
            {
	      int eN_j=eN*nDOF_trial_element+j;
	      int eN_k_j=eN_k*nDOF_trial_element+j;
	      int eN_k_j_nSpace = eN_k_j*nSpace;
              
              u += valFromDOF_c(u_dof[u_l2g[eN_j]],u_trial[eN_k_j]);
	      for (int I=0;I<nSpace;I++)
		{
		  grad_u[I] += gradFromDOF_c(u_dof[u_l2g[eN_j]],u_grad_trial[eN_k_j_nSpace+I]);
		}
	    }
          //
          //calculate pde coefficients and derivatives at quadrature points
          //
          evaluateCoefficients_c(epsilon_redist,
				 phi_ls[eN_k],
				 u,
				 grad_u,
				 m,
				 dm,
				 H,
				 dH,
				 r);
	  //TODO allow not lagging of subgrid error etc
	  //remove conditional?
	  //default no lagging
	  for (int I=0; I < nSpace; I++)
	    {
	      dH_tau[I] = dH[I];
	      dH_strong[I] = dH[I];
	    }
	  if (lag_subgridError > 0)
	    {
	      for (int I=0; I < nSpace; I++)
		{
		  dH_tau[I] = q_dH_last[eN_k_nSpace+I];
		}
	    }
	  if (lag_subgridError > 1)
	    {
	      for (int I=0; I < nSpace; I++)
		{
		  dH_strong[I] = q_dH_last[eN_k_nSpace+I];
		}
	    }
// 	  if (lag_subgridError)
// 	    {
// 	      for (int I=0; I < nSpace; I++)
// 		{
// 		  dH_sge[I] = q_dH_last[eN_k_nSpace+I];
// 		}
// 	    }
// 	  else
// 	    {
// 	      for (int I=0; I < nSpace; I++)
// 		{
// 		  dH_sge[I] = dH[I];
// 		}
// 	    }
          //
          //moving mesh
          //
          //omit for now
          //
          //calculate time derivatives
          //
          bdf_c(alphaBDF,
		q_m_betaBDF[eN_k],
		m,
		dm,
		m_t,
		dm_t);
	  //TODO add option to skip if not doing time integration (Newton stead-state solve)
	  m *= timeIntegrationScale; dm *= timeIntegrationScale; m_t *= timeIntegrationScale;
	  dm_t *= timeIntegrationScale;

          //
          //calculate subgrid error contribution to the Jacobian (strong residual, adjoint, jacobian of strong residual)
          //
          //calculate the adjoint times the test functions
          for (int i=0;i<nDOF_test_element;i++)
            {
	      int eN_k_i_nSpace = (eN_k*nDOF_trial_element+i)*nSpace;

	      Lstar_u[i]=Hamiltonian_adjoint_c(dH_strong,&u_grad_test_dV[eN_k_i_nSpace]);
	      
            }
          //calculate the Jacobian of strong residual
          for (int j=0;j<nDOF_trial_element;j++)
            {
	      int eN_k_j=eN_k*nDOF_trial_element+j;
	      int eN_k_j_nSpace = eN_k_j*nSpace;
	      dpdeResidual_u_u[j]=MassJacobian_strong_c(dm_t,u_trial[eN_k_j]) +
		HamiltonianJacobian_strong_c(dH_strong,&u_grad_trial[eN_k_j_nSpace]);

            }
          //tau and tau*Res
          calculateSubgridError_tau_c(elementDiameter[eN],
				      dm_t,
				      dH_tau,
				      q_cfl[eN_k],
				      tau);
          calculateSubgridErrorDerivatives_tauRes_c(tau,
						    dpdeResidual_u_u,
						    dsubgridError_u_u);

	  nu_sc = q_numDiff_u[eN_k]*(1.0-lag_shockCapturingScale) + q_numDiff_u_last[eN_k]*lag_shockCapturingScale;

	  //mwf debug
//  	  if (dH[0]*dH_sge[0] <= 0.0)
// 	    {
// 	      std::cout<<"RDLSQ jac eN= "<<eN<<" u= "<<u<<" m= "<<m<<" alphaBDF= "<<alphaBDF<<" m_t= "<<m_t<<" dm_t= "<<dm_t
// 		       <<" r= "<<r<<" dH[0]= "<< dH[0]<<" dH_sge[0]= "<<dH_sge[0]<<" grad_u[0]="<<grad_u[0] 
// 		       <<" m_last= "<<q_m_betaBDF[eN_k]
// 		       <<"\n\t tau= "<<tau<<" dpdeResidual[0]= "<<dpdeResidual_u_u[0] <<" dsubgridError_u_u[0]= "<<dsubgridError_u_u[0]
// 		       <<" numDiff_last= "<<q_numDiff_u_last[eN_k]<<std::endl;
// 	    }
 	  for(int i=0;i<nDOF_test_element;i++)
	    {
	      int eN_k_i=eN_k*nDOF_test_element+i;
	      int eN_k_i_nSpace=eN_k_i*nSpace;
	      for(int j=0;j<nDOF_trial_element;j++) 
		{ 
		  int eN_k_j=eN_k*nDOF_trial_element+j;
		  int eN_k_j_nSpace = eN_k_j*nSpace;

		  elementJacobian_u_u[i][j] += MassJacobian_weak_c(dm_t,u_trial[eN_k_j],u_test_dV[eN_k_i]) + 
		    HamiltonianJacobian_weak_c(dH,&u_grad_trial[eN_k_j_nSpace],u_test_dV[eN_k_i]) +
		    SubgridErrorJacobian_c(dsubgridError_u_u[j],Lstar_u[i]) + 
		    NumericalDiffusionJacobian_c(nu_sc,&u_grad_trial[eN_k_j_nSpace],&u_grad_test_dV[eN_k_i_nSpace]); 
		  
		}//j
            }//i
	}//k
      //
      //load into element Jacobian into global Jacobian
      //
      
      //now try to account for weak dirichlet conditions in interior (frozen level set values)
      if (freezeLevelSet)
	{
	  const double weakDirichletFactor = 3.0;
	  //assume correspondence between dof and equations 
	  for (int j = 0; j < nDOF_trial_element; j++)
	    {
	      const int J = u_l2g[eN*nDOF_trial_element+j];
	      if (weakDirichletConditionFlags[J] == 1)
		{
		  for (int jj=0; jj < nDOF_trial_element; jj++)
		    elementJacobian_u_u[j][jj] = 0.0;
		  elementJacobian_u_u[j][j] = weakDirichletFactor*elementDiameter[eN];
		  //mwf debug
		  //std::cout<<"RDLSQ Jac freeze eN= "<<eN<<" j= "<<j<<" J= "<<J<<std::endl;
		}
	    }
	}
      for (int i=0;i<nDOF_test_element;i++)
	{
	  int eN_i = eN*nDOF_test_element+i;
	  for (int j=0;j<nDOF_trial_element;j++)
	    {
	      int eN_i_j = eN_i*nDOF_trial_element+j;
	      globalJacobian[csrRowIndeces_u_u[eN_i] + csrColumnOffsets_u_u[eN_i_j]] += elementJacobian_u_u[i][j];
	    }//j
	}//i
    }//elements
  //
  //loop over exterior element boundaries to compute the surface integrals and load them into the global Jacobian
  //
  for (int ebNE = 0; ebNE < nExteriorElementBoundaries_global; ebNE++) 
    { 
      register int ebN = exteriorElementBoundariesArray[ebNE]; 
      register int eN  = elementBoundaryElementsArray[ebN*2+0];
      const double epsilon_redist= epsFact_redist*elementDiameter[eN];
      for  (int kb=0;kb<nQuadraturePoints_elementBoundary;kb++) 
	{ 
	  register int ebNE_kb = ebNE*nQuadraturePoints_elementBoundary+kb,
	    ebNE_kb_nSpace = ebNE_kb*nSpace;

	  register double u_ext=0.0,
	    grad_u_ext[nSpace],
	    m_ext=0.0,
	    dm_ext=0.0,
	    H_ext=0.0,
	    dH_ext[nSpace],
	    r_ext=0.0,
	    flux_ext=0.0,
	    dflux_u_u_ext=0.0,
	    bc_u_ext=0.0,
	    bc_grad_u_ext[nSpace],
	    bc_m_ext=0.0,
	    bc_dm_ext=0.0,
	    bc_H_ext=0.0,
	    bc_dH_ext[nSpace],
	    bc_r_ext=0.0,
	    fluxJacobian_u_u[nDOF_trial_element];
	  // 
	  //calculate the solution and gradients at quadrature points 
	  // 
	  u_ext=0.0;
	  for (int I=0;I<nSpace;I++)
	    {
	      grad_u_ext[I] = 0.0;
	      bc_grad_u_ext[I] = 0.0;
	    }
	  for (int j=0;j<nDOF_trial_element;j++) 
	    { 
	      register int eN_j = eN*nDOF_trial_element+j,
		ebNE_kb_j = ebNE_kb*nDOF_trial_element+j,
		ebNE_kb_j_nSpace= ebNE_kb_j*nSpace;
	      u_ext += valFromDOF_c(u_dof[u_l2g[eN_j]],u_trial_ext[ebNE_kb_j]); 
	                     
	      for (int I=0;I<nSpace;I++)
		{
		  grad_u_ext[I] += gradFromDOF_c(u_dof[u_l2g[eN_j]],u_grad_trial_ext[ebNE_kb_j_nSpace+I]); 
		} 
	    }
	  //
	  //load the boundary values
	  //
	  bc_u_ext = isDOFBoundary_u[ebNE_kb]*ebqe_bc_u_ext[ebNE_kb]+(1-isDOFBoundary_u[ebNE_kb])*u_ext;
	  // 
	  //calculate the internal and external trace of the pde coefficients 
	  // 
	  evaluateCoefficients_c(epsilon_redist,
				 ebqe_phi_ls_ext[ebNE_kb],
				 u_ext,
				 grad_u_ext,
				 m_ext,
				 dm_ext,
				 H_ext,
				 dH_ext,
				 r_ext);
	  evaluateCoefficients_c(epsilon_redist,
				 ebqe_phi_ls_ext[ebNE_kb],
				 bc_u_ext,
				 bc_grad_u_ext,
				 bc_m_ext,
				 bc_dm_ext,
				 bc_H_ext,
				 bc_dH_ext,
				 bc_r_ext);
	  // 
	  //calculate the numerical fluxes 
	  // 
	  //DoNothing for now
	  //
	  //calculate the flux jacobian
	  //
	  for (int j=0;j<nDOF_trial_element;j++)
	    {
	      register int ebNE_kb_j = ebNE_kb*nDOF_trial_element+j,
		ebNE_kb_j_nSpace = ebNE_kb_j*nSpace;

	      fluxJacobian_u_u[j]=ExteriorNumericalAdvectiveFluxJacobian_c(dflux_u_u_ext,u_trial_ext[ebNE_kb_j]);
	    }//j
	  //
	  //update the global Jacobian from the flux Jacobian
	  //
	  for (int i=0;i<nDOF_test_element;i++)
	    {
	      register int eN_i = eN*nDOF_test_element+i,
		ebNE_kb_i = ebNE_kb*nDOF_test_element+i;
	      for (int j=0;j<nDOF_trial_element;j++)
		{
		  register int ebN_i_j = ebN*4*nDOF_test_X_trial_element + i*nDOF_trial_element + j;
		  //mwf debug
		  assert(fluxJacobian_u_u[j] == 0.0);
		  globalJacobian[csrRowIndeces_u_u[eN_i] + csrColumnOffsets_eb_u_u[ebN_i_j]] += fluxJacobian_u_u[j]*u_test_dS_ext[ebNE_kb_i];
		}//j
	    }//i
	}//kb
    }//ebNE

}//computeJacobian

