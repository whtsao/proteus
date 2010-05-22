#ifndef SUBGRIDERROR_H
#define SUBGRIDERROR_H
/**
 \file subgridError.h
 \brief C implementations of subgrid error calculations
*/

/**
 \defgroup subgridError subgridError
 \brief c implementation of multiscale fem subgrid error approximations
 @{
*/

extern void calculateSubgridError_tauRes(
  int nElements_global,
  int nQuadraturePoints_element,
  int nDOF_trial_element,
  double *tau,
  double *pdeResidual,
  double *dpdeResidual,
  double *subgridError,
  double *dsubgridError
);
extern void calculateSubgridError_ADR_generic_tau(int nElements_global,
                                           int nQuadraturePoints_element,
                                           int nSpace,
                                           double* inverseJ,
                                           double* dmt,
                                           double* df,
                                           double* a,
                                           double* da,
                                           double* grad_phi,
                                           double* dphi,
                                           double* dr,
                                           double* pe,
                                           double* cfl,
                                           double* tau);
extern void calculateSubgridErrorNavierStokes2D_generic_tau(int nElements_global,
                                                     int nQuadraturePoints_element,
                                                     int nSpace,
                                                     double* inverseJ,
                                                     double* dmt,
                                                     double* dm,
                                                     double* f,
                                                     double* a,
                                                     double* tau0,
                                                     double* tau1,
                                                     double* cfl);
extern void calculateSubgridError_ADR_tau_p(
  int nElements_global,
  int nQuadraturePoints_element,
  int nSpace,
  double *elementDiameter,
  double *dmt,
  double *df,
  double *a,
  double *da,
  double *grad_phi,
  double *dphi,
  double *dr,
  double *pe,
  double *cfl,
  double *tau
);
extern void calculateSubgridError_ADR_tau_1(
  int nElements_global,
  int nQuadraturePoints_element,
  int nSpace,
  double *elementDiameter,
  double *dmt,
  double *df,
  double *a,
  double *da,
  double *grad_phi,
  double *dphi,
  double *dr,
  double *pe,
  double *cfl,
  double *tau
);
extern void calculateSubgridError_ADR_tau_2(
  int nElements_global,
  int nQuadraturePoints_element,
  int nSpace,
  double *elementDiameter,
  double *dmt,
  double *df,
  double *a,
  double *da,
  double *grad_phi,
  double *dphi,
  double *dr,
  double *pe,
  double *cfl,
  double *tau
);
extern void calculateSubgridError_ADR_tau(
  int nElements_global,
  int nQuadraturePoints_element,
  int nSpace,
  char stabilization,
  double *elementDiameter,
  double *dmt,
  double *df,
  double *a,
  double *da,
  double *grad_phi,
  double *dphi,
  double *dr,
  double *pe,
  double *cfl,
  double *tau
);
extern void calculateSubgridError_A_tau_1(
  int nElements_global,
  int nQuadraturePoints_element,
  int nSpace,
  double *elementDiameter,
  double *dmt,
  double *df,
  double *cfl,
  double *tau
);
extern void calculateSubgridError_A_tau_2(
  int nElements_global,
  int nQuadraturePoints_element,
  int nSpace,
  double *elementDiameter,
  double *dmt,
  double *df,
  double *cfl,
  double *tau
);
extern void calculateSubgridError_A_tau(
  int nElements_global,
  int nQuadraturePoints_element,
  int nSpace,
  char stabilization,
  double *elementDiameter,
  double *dmt,
  double *df,
  double *cfl,
  double *tau
);
extern void calculateSubgridErrorStokes2D_GLS_velocity(
  int nElements_global,
  int nQuadraturePoints_element,
  int nDOF_trial_element,
  int nSpace,
  double *elementDiameter,
  double *a,
  double *pdeResidualU,
  double *dpdeResidualU_dp,
  double *dpdeResidualU_du,
  double *pdeResidualV,
  double *dpdeResidualV_dp,
  double *dpdeResidualV_dv,
  double *subgridErrorU,
  double *dsubgridErrorU_dp,
  double *dsubgridErrorU_du,
  double *subgridErrorV,
  double *dsubgridErrorV_dp,
  double *dsubgridErrorV_dv
);
extern void calculateSubgridErrorStokes3D_GLS_velocity(
  int nElements_global,
  int nQuadraturePoints_element,
  int nDOF_trial_element,
  int nSpace,
  double *elementDiameter,
  double *a,
  double *pdeResidualU,
  double *dpdeResidualU_dp,
  double *dpdeResidualU_du,
  double *pdeResidualV,
  double *dpdeResidualV_dp,
  double *dpdeResidualV_dv,
  double *pdeResidualW,
  double *dpdeResidualW_dp,
  double *dpdeResidualW_dw,
  double *subgridErrorU,
  double *dsubgridErrorU_dp,
  double *dsubgridErrorU_du,
  double *subgridErrorV,
  double *dsubgridErrorV_dp,
  double *dsubgridErrorV_dv,
  double *subgridErrorW,
  double *dsubgridErrorW_dp,
  double *dsubgridErrorW_dw
);
extern void calculateSubgridErrorStokes2D_GLS_velocity_pressure(
  int nElements_global,
  int nQuadraturePoints_element,
  int nDOF_trial_element,
  int nSpace,
  double *elementDiameter,
  double *a,
  double *pdeResidualP,
  double *dpdeResidualP_du,
  double *dpdeResidualP_dv,
  double *pdeResidualU,
  double *dpdeResidualU_dp,
  double *dpdeResidualU_du,
  double *pdeResidualV,
  double *dpdeResidualV_dp,
  double *dpdeResidualV_dv,
  double *subgridErrorP,
  double *dsubgridErrorP_du,
  double *dsubgridErrorP_dv,
  double *subgridErrorU,
  double *dsubgridErrorU_dp,
  double *dsubgridErrorU_du,
  double *subgridErrorV,
  double *dsubgridErrorV_dp,
  double *dsubgridErrorV_dv
);
extern void calculateSubgridErrorStokes3D_GLS_velocity_pressure(
  int nElements_global,
  int nQuadraturePoints_element,
  int nDOF_trial_element,
  int nSpace,
  double *elementDiameter,
  double *a,
  double *pdeResidualP,
  double *dpdeResidualP_du,
  double *dpdeResidualP_dv,
  double *dpdeResidualP_dw,
  double *pdeResidualU,
  double *dpdeResidualU_dp,
  double *dpdeResidualU_du,
  double *pdeResidualV,
  double *dpdeResidualV_dp,
  double *dpdeResidualV_dv,
  double *pdeResidualW,
  double *dpdeResidualW_dp,
  double *dpdeResidualW_dw,
  double *subgridErrorP,
  double *dsubgridErrorP_du,
  double *dsubgridErrorP_dv,
  double *dsubgridErrorP_dw,
  double *subgridErrorU,
  double *dsubgridErrorU_dp,
  double *dsubgridErrorU_du,
  double *subgridErrorV,
  double *dsubgridErrorV_dp,
  double *dsubgridErrorV_dv,
  double *subgridErrorW,
  double *dsubgridErrorW_dp,
  double *dsubgridErrorW_dw
);
extern void calculateSubgridErrorNavierStokes2D_GLS_tau(
  int nElements_global,
  int nQuadraturePoints_element,
  int nSpace,
  double hFactor,
  double *elementDiameter,
  double *dmt,
  double *dm,
  double *f,
  double *a,
  double *tau0,
  double *tau1,
  double *cfl
);
extern void calculateSubgridErrorStokes2D_GLS_tau(
  int nElements_global,
  int nQuadraturePoints_element,
  int nSpace,
  double *elementDiameter,
  double *dmt,
  double *a,
  double *tau0,
  double *tau1
);
extern void calculateSubgridErrorStokes2D_GLS_tauRes(
  int nElements_global,
  int nQuadraturePoints_element,
  int nDOF_trial_element,
  int nSpace,
  double *tau0,
  double *tau1,
  double *pdeResidualP,
  double *dpdeResidualP_du,
  double *dpdeResidualP_dv,
  double *pdeResidualU,
  double *dpdeResidualU_dp,
  double *dpdeResidualU_du,
  double *pdeResidualV,
  double *dpdeResidualV_dp,
  double *dpdeResidualV_dv,
  double *subgridErrorP,
  double *dsubgridErrorP_du,
  double *dsubgridErrorP_dv,
  double *subgridErrorU,
  double *dsubgridErrorU_dp,
  double *dsubgridErrorU_du,
  double *subgridErrorV,
  double *dsubgridErrorV_dp,
  double *dsubgridErrorV_dv
);

extern void calculateSubgridErrorStokes3D_GLS_tauRes(int nElements_global,
						     int nQuadraturePoints_element,
						     int nDOF_trial_element,
						     int nSpace,
						     double* tau0,
						     double* tau1,
						     double* pdeResidualP,
						     double* dpdeResidualP_du,
						     double* dpdeResidualP_dv,
						     double* dpdeResidualP_dw,
						     double* pdeResidualU,
						     double* dpdeResidualU_dp,
						     double* dpdeResidualU_du,
						     double* pdeResidualV,
						     double* dpdeResidualV_dp,
						     double* dpdeResidualV_dv,
						     double* pdeResidualW,
						     double* dpdeResidualW_dp,
						     double* dpdeResidualW_dw,
						     double* subgridErrorP,
						     double* dsubgridErrorP_du,
						     double* dsubgridErrorP_dv,
						     double* dsubgridErrorP_dw,
						     double* subgridErrorU,
						     double* dsubgridErrorU_dp,
						     double* dsubgridErrorU_du,
						     double* subgridErrorV,
						     double* dsubgridErrorV_dp,
						     double* dsubgridErrorV_dv,
						     double* subgridErrorW,
						     double* dsubgridErrorW_dp,
						     double* dsubgridErrorW_dw);


extern void calculateSubgridErrorNavierStokes3D_GLS_velocity_pressure(
  int nElements_global,
  int nQuadraturePoints_element,
  int nDOF_trial_element,
  int nSpace,
  double *elementDiameter,
  double *dm,
  double *f,
  double *a,
  double *pdeResidualP,
  double *dpdeResidualP_du,
  double *dpdeResidualP_dv,
  double *dpdeResidualP_dw,
  double *pdeResidualU,
  double *dpdeResidualU_dp,
  double *dpdeResidualU_du,
  double *pdeResidualV,
  double *dpdeResidualV_dp,
  double *dpdeResidualV_dv,
  double *pdeResidualW,
  double *dpdeResidualW_dp,
  double *dpdeResidualW_dw,
  double *subgridErrorP,
  double *dsubgridErrorP_du,
  double *dsubgridErrorP_dv,
  double *dsubgridErrorP_dw,
  double *subgridErrorU,
  double *dsubgridErrorU_dp,
  double *dsubgridErrorU_du,
  double *subgridErrorV,
  double *dsubgridErrorV_dp,
  double *dsubgridErrorV_dv,
  double *subgridErrorW,
  double *dsubgridErrorW_dp,
  double *dsubgridErrorW_dw
);
extern void calculateSubgridErrorStokes2D_1(
  int nElements_global,
  int nQuadraturePoints_element,
  int nDOF_trial_element,
  int nSpace,
  double *elementDiameter,
  double *u,
  double *v,
  double *a,
  double *pdeResidualP,
  double *dpdeResidualP_du,
  double *dpdeResidualP_dv,
  double *pdeResidualU,
  double *dpdeResidualU_dp,
  double *dpdeResidualU_du,
  double *pdeResidualV,
  double *dpdeResidualV_dp,
  double *dpdeResidualV_dv,
  double *subgridErrorP,
  double *dsubgridErrorP_dp,
  double *dsubgridErrorP_du,
  double *dsubgridErrorP_dv,
  double *subgridErrorU,
  double *dsubgridErrorU_dp,
  double *dsubgridErrorU_du,
  double *dsubgridErrorU_dv,
  double *subgridErrorV,
  double *dsubgridErrorV_dp,
  double *dsubgridErrorV_du,
  double *dsubgridErrorV_dv
);
/** @} */
#endif
