/*
  Kirchhoff-Love shell implementation in PetIGA
  author: Shiva Rudraraju
 */

#include "petiga.h"
#include <math.h> 

//include automatic differentiation library
#include <Sacado.hpp>
typedef Sacado::Fad::DFad<double> doubleAD;

#define LagrangeMultiplierMethod

#include "include/residual.h"
#include "include/output.h"
#include "include/solvers.h"

//parameters
#define bvpType 0
#define numLoadSteps 100 

#undef  __FUNCT__
#define __FUNCT__ "setupBVP"
PetscErrorCode setupBVP(BVPStruct& bvp)
{
  PetscFunctionBegin;
  PetscErrorCode ierr;
  
  //Boundary form for Neumann BC's
  IGAForm form;
  ierr = IGAGetForm(bvp.iga,&form);CHKERRQ(ierr);
  ierr = IGAFormSetBoundaryForm (form,0,0,PETSC_FALSE);CHKERRQ(ierr);
  ierr = IGAFormSetBoundaryForm (form,0,1,PETSC_FALSE);CHKERRQ(ierr);
  ierr = IGAFormSetBoundaryForm (form,1,0,PETSC_FALSE);CHKERRQ(ierr);
  ierr = IGAFormSetBoundaryForm (form,1,1,PETSC_FALSE);CHKERRQ(ierr);
  bvp.angleConstraints[0]= bvp.angleConstraints[1]=false;

  //Dirichlet and Neumann BC's
  switch (bvp.type) {
  case 0: //cap BVP
    //Dirichlet
    ierr = IGASetBoundaryValue(bvp.iga,0,0,1,0.0);CHKERRQ(ierr); //Y=0 at the bottom of the cap
    ierr = IGASetBoundaryValue(bvp.iga,0,1,0,0.0);CHKERRQ(ierr); //X=0 at the top of the cap
    ierr = IGASetBoundaryValue(bvp.iga,0,1,2,0.0);CHKERRQ(ierr); //Z=0 at the top of the cap
    bvp.uDirichlet=bvp.l*1.0e-1; //X=Z=uDirichlet at the bottom of the cap (Pinching through displacement control)
    
    //Neumann
    ierr = IGAFormSetBoundaryForm (form,0,0,PETSC_TRUE);CHKERRQ(ierr); //phi=90 at the bottom of the cap
    ierr = IGAFormSetBoundaryForm (form,0,1,PETSC_TRUE);CHKERRQ(ierr); //phi=0  at the top of the cap
    bvp.angleConstraints[0]=true; bvp.angleConstraintValues[0]=90;
    bvp.angleConstraints[1]=true; bvp.angleConstraintValues[1]=0;
    break;
    
  case 1: //tube BVP
    //Dirichlet
    ierr = IGASetBoundaryValue(bvp.iga,0,0,1,0.0);CHKERRQ(ierr); //Y=0 at the bottom of the tube
    ierr = IGASetBoundaryValue(bvp.iga,0,1,0,0.0);CHKERRQ(ierr); //X=0 at the top of the tube
    ierr = IGASetBoundaryValue(bvp.iga,0,1,2,0.0);CHKERRQ(ierr); //Z=0 at the top of the tube
    bvp.uDirichlet=bvp.l*1.0e-1; //X=Z=uDirichlet at the bottom of the tube (Pinching through displacement control)
    
    //Neumann
    ierr = IGAFormSetBoundaryForm (form,0,0,PETSC_TRUE);CHKERRQ(ierr); //phi=90 at the bottom of the tube
    ierr = IGAFormSetBoundaryForm (form,0,1,PETSC_TRUE);CHKERRQ(ierr); //phi=90 at the top of the tube
    bvp.angleConstraints[0]=true; bvp.angleConstraintValues[0]=90;
    bvp.angleConstraints[1]=true; bvp.angleConstraintValues[1]=90;
    break;
    
  case 2: //base BVP
    //Dirichlet
    ierr = IGASetBoundaryValue(bvp.iga,0,0,0,0.0);CHKERRQ(ierr); //X=0 at the bottom of the base
    ierr = IGASetBoundaryValue(bvp.iga,0,0,1,0.0);CHKERRQ(ierr); //Y=0 at the bottom of the base
    ierr = IGASetBoundaryValue(bvp.iga,0,0,2,0.0);CHKERRQ(ierr); //Z=0 at the bottom of the base
    //ierr = IGASetBoundaryValue(bvp.iga,0,1,1,0.0);CHKERRQ(ierr); //Y=0 at the top of the base
    bvp.uDirichlet=bvp.l*1.0e-1; //X=Z=uDirichlet at the bottom of the tube (Pinching through displacement control)
    
    //Neumann
    ierr = IGAFormSetBoundaryForm (form,0,0,PETSC_TRUE);CHKERRQ(ierr); //phi=0 at the bottom of the base
    ierr = IGAFormSetBoundaryForm (form,0,1,PETSC_TRUE);CHKERRQ(ierr); //phi=90 at the top of the base
    bvp.angleConstraints[0]=true; bvp.angleConstraintValues[0]=0;
    bvp.angleConstraints[1]=true; bvp.angleConstraintValues[1]=90;
    break;

  case 3: //pulling flat membrane BVP
    break;
  }
  
  PetscFunctionReturn(0);
}

//main
int main(int argc, char *argv[]) {
  PetscErrorCode ierr;
  ierr = PetscInitialize(&argc,&argv,0,0);CHKERRQ(ierr);

  //IGA and BVP setup
  IGA iga;
  ierr = IGACreate(PETSC_COMM_WORLD,&iga);CHKERRQ(ierr);
  
  //setup BVP and material model parameters
  BVPStruct bvp;
  bvp.kMean=1.0;
  bvp.kGaussian=0.0;
  bvp.mu=0.01;
  bvp.lambda=1000;
  bvp.l=1.0;
  bvp.epsilon=1.0;
  bvp.type=bvpType;
  
#ifdef LagrangeMultiplierMethod
  ierr = IGASetDof(iga,4);CHKERRQ(ierr); // dofs = {ux,uy,uz,q}
#else
  ierr = IGASetDof(iga,3);CHKERRQ(ierr); // dofs = {ux,uy,uz}
#endif
  ierr = IGASetDim(iga,2);CHKERRQ(ierr);
  ierr = IGASetGeometryDim(iga,3);CHKERRQ(ierr);
  ierr = IGAAxisSetPeriodic(iga->axis[1],PETSC_TRUE);CHKERRQ(ierr);
  switch (bvp.type) {
  case 0: //cap BVP
    ierr = IGARead(iga,"meshes/capMesh.dat"); CHKERRQ(ierr); break;
  case 1: //tube BVP
    ierr = IGARead(iga,"meshes/tubeMesh.dat"); CHKERRQ(ierr); break;
  case 2: //base BVP
    ierr = IGARead(iga,"meshes/baseMesh.dat"); CHKERRQ(ierr); break;
  }
  ierr = IGASetFromOptions(iga);CHKERRQ(ierr);
  ierr = IGASetUp(iga);CHKERRQ(ierr);
  bvp.iga = iga;
  //
  setupBVP(bvp);
  
  //Print knots to output
  PetscMPIInt rank,size;
  ierr = MPI_Comm_rank(PETSC_COMM_WORLD,&rank);CHKERRQ(ierr);
  ierr = MPI_Comm_size(PETSC_COMM_WORLD,&size);CHKERRQ(ierr);
  IGAAxis axisX;  
  ierr = IGAGetAxis(bvp.iga,0,&axisX);CHKERRQ(ierr);
  PetscInt mX; PetscReal* UX;
  IGAAxisGetKnots(axisX, &mX, &UX);
  IGAAxis axisY;  
  ierr = IGAGetAxis(bvp.iga,1,&axisY);CHKERRQ(ierr);
  PetscInt mY; PetscReal* UY;
  IGAAxisGetKnots(axisY, &mY, &UY);
  if(rank == size-1){
    std::cout << mX << " knotsX: ";
    for (unsigned int i=0; i<(mX+1); i++){
      std::cout << UX[i] << ", ";
    }
    std::cout << std::endl;
    std::cout << mY << " knotsY: ";
    for (unsigned int i=0; i<(mY+1); i++){
      std::cout << UY[i] << ", ";
    }
    std::cout << std::endl;
  }
  
  
  //solutions vectors
  Vec U;
  ierr = IGACreateVec(bvp.iga,&U);CHKERRQ(ierr);
  ierr = VecSet(U,0.0);CHKERRQ(ierr);
  ierr = IGACreateVec(bvp.iga,&bvp.xDirichlet);CHKERRQ(ierr);
  
  //
  ierr = IGASetFormIEFunction(bvp.iga,Residual,&bvp);CHKERRQ(ierr);
  ierr = IGASetFormIEJacobian(bvp.iga,Jacobian,&bvp);CHKERRQ(ierr);

  //load stepping
  TS ts;
  PetscInt timeSteps=numLoadSteps;
  ierr = IGACreateTS(bvp.iga,&ts);CHKERRQ(ierr);
  ierr = TSSetType(ts,TSBEULER);CHKERRQ(ierr);
  //ierr = TSSetMaxSteps(ts,timeSteps+1);CHKERRQ(ierr);
  ierr = TSSetMaxTime(ts, 1.01);
  ierr = TSSetExactFinalTime(ts,TS_EXACTFINALTIME_STEPOVER);CHKERRQ(ierr);
  ierr = TSSetTime(ts,0.0);CHKERRQ(ierr);
  ierr = TSSetTimeStep(ts,1.0/timeSteps);CHKERRQ(ierr);
  ierr = TSMonitorSet(ts,OutputMonitor,&bvp,NULL);CHKERRQ(ierr);
  ierr = TSSetFromOptions(ts);CHKERRQ(ierr);
  SNES snes;
  TSGetSNES(ts,&snes);
  SNESSetConvergenceTest(snes,SNESConverged_Interactive,(void*)&bvp,NULL);
#if PETSC_VERSION_LE(3,3,0)
  ierr = TSSolve(ts,U,NULL);CHKERRQ(ierr);
#else
  ierr = TSSolve(ts,U);CHKERRQ(ierr);
#endif

  //
  ierr = VecDestroy(&U);CHKERRQ(ierr);
  ierr = TSDestroy(&ts);CHKERRQ(ierr);
  ierr = IGADestroy(&bvp.iga);CHKERRQ(ierr);
  ierr = PetscFinalize();CHKERRQ(ierr);
  return 0;
}
