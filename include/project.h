/*
  L2 Projection functions
  author: Shiva Rudraraju
 */

#if !defined(PROJECT_H_)
#define PROJECT_H_

#undef __FUNCT__
#define __FUNCT__ "FunctionL2"
PetscErrorCode FunctionDirichletL2(IGAPoint p, const PetscScalar *U, PetscScalar *R, void *ctx)
{
  PetscFunctionBegin;
  PetscErrorCode ierr;
  BVPStruct *bvp = (BVPStruct *)ctx;
  
  PetscInt nen, dof;
  IGAPointGetSizes(p,0,&nen,&dof);
  PetscReal x[3];
  IGAPointFormGeomMap(p,x);

  //normal direction in XZ plane
  PetscReal n[3];
  if ((x[0]*x[0]+x[2]*x[2])>0.0){
    n[0]=x[0]/sqrt(x[0]*x[0]+x[2]*x[2]);
    n[1]=0.0;
    n[2]=x[2]/sqrt(x[0]*x[0]+x[2]*x[2]);
  }
  else{
    n[0]=n[1]=n[2]=0.0;
  }
	
  PetscReal uDirichletVal=bvp->uDirichlet;  
  if (x[1]>(0.1*bvp->l)){ uDirichletVal=0.0;}//for top surface

  //L2 projection residual
  const PetscReal (*N) = (const PetscReal (*)) p->shape[0];;  
  for(int n1=0; n1<nen; n1++){
    for(int d1=0; d1<dof; d1++){
      PetscReal val=0.0;
      switch (d1) {
      case 0:
	val=uDirichletVal*n[0]; break;
      case 1:
	val=0.0; break;
      case 2:
	val=uDirichletVal*n[2]; break;
      case 3:
	val=0.0; break;
      }
      R[n1*dof+d1] = N[n1]*val;
    }
  }
  
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "FunctionR"
PetscErrorCode FunctionR(IGAPoint p, const PetscScalar *U, PetscScalar *R, void *ctx)
{
  PetscFunctionBegin;
  Residual(p, 0, 0, 0, U, 0, U, R, ctx);
  PetscFunctionReturn(0);
}


#undef __FUNCT__
#define __FUNCT__ "JacobianL2"
PetscErrorCode JacobianL2(IGAPoint p, const PetscScalar *U, PetscScalar *K, void *ctx)
{
  PetscFunctionBegin;
  PetscErrorCode ierr;
  BVPStruct *bvp = (BVPStruct *)ctx;
  
  PetscInt dim = p->dim;
  PetscInt nen, dof;
  IGAPointGetSizes(p,0,&nen,&dof);
  
  const PetscReal *N = (const PetscReal (*)) p->shape[0];

  for(int n1=0; n1<nen; n1++){
    for(int d1=0; d1<dof; d1++){
      for(int n2=0; n2<nen; n2++){
	for(int d2=0; d2<dof; d2++){
	  PetscReal val2=0.0;
	  if (d1==d2) {val2 = N[n1] * N[n2];}
	  K[n1*dof*nen*dof + d1*nen*dof + n2*dof + d2] =val2;
	}
      }
    }
  }

  return 0;
}


#undef __FUNCT__
#define __FUNCT__ "ProjectL2"
PetscErrorCode ProjectL2(void *ctx)
{
  PetscFunctionBegin;
  PetscErrorCode ierr;
  BVPStruct *bvp = (BVPStruct *)ctx;
  
  //Solve L2 projection problem
  bvp->projectBC=true;
  Mat A;
  Vec b;
  ierr = VecSet(bvp->xDirichlet,0.0);CHKERRQ(ierr);
  ierr = IGACreateMat(bvp->iga,&A);CHKERRQ(ierr);
  ierr = IGACreateVec(bvp->iga,&b);CHKERRQ(ierr);
  ierr = MatSetOption(A,MAT_SYMMETRIC,PETSC_TRUE);CHKERRQ(ierr);
  ierr = IGASetFormFunction(bvp->iga,FunctionDirichletL2,bvp);CHKERRQ(ierr);
  ierr = IGASetFormJacobian(bvp->iga,JacobianL2,bvp);CHKERRQ(ierr);
  ierr = IGAComputeFunction(bvp->iga,bvp->xDirichlet,b);CHKERRQ(ierr);
  ierr = IGAComputeJacobian(bvp->iga,bvp->xDirichlet,A);CHKERRQ(ierr);
  //Solver
  {
    KSP ksp;
    ierr = IGACreateKSP(bvp->iga,&ksp);CHKERRQ(ierr);
    ierr = KSPSetOperators(ksp,A,A);CHKERRQ(ierr);
    ierr = KSPSetFromOptions(ksp);CHKERRQ(ierr);
    ierr = KSPSolve(ksp,b,bvp->xDirichlet);CHKERRQ(ierr);
    ierr = KSPDestroy(&ksp);CHKERRQ(ierr);
  }
  /*
  char           filename[256];
  sprintf(filename,"./UU%d.vts",bvp->load_increment);
  ierr = IGADrawVecVTK(bvp->iga,bvp->xDirichlet,filename);CHKERRQ(ierr);
  */
  ierr = VecDestroy(&b);CHKERRQ(ierr);
  ierr = MatDestroy(&A);CHKERRQ(ierr);
  
  PetscFunctionReturn(0);
}


#undef __FUNCT__
#define __FUNCT__ "ProjectR"
PetscErrorCode ProjectR(Vec& U, void *ctx)
{
  PetscFunctionBegin;
  PetscErrorCode ierr;
  BVPStruct *bvp = (BVPStruct *)ctx;

  //
  ierr = IGASetFixTable(bvp->iga,NULL);CHKERRQ(ierr); /* Clear vector to read BCs from */
  //clear old BC's
  IGAForm form;
  ierr = IGAGetForm(bvp->iga,&form);CHKERRQ(ierr);
  for (PetscInt dir=0; dir<2; dir++){
    for (PetscInt side=0; side<2; side++){
      ierr =   IGAFormClearBoundary(form,dir,side);
    }
  }
  
  //Solve L2 projection problem
  Mat A;
  Vec b;
  ierr = VecSet(bvp->xDirichlet,0.0);CHKERRQ(ierr);
  ierr = IGACreateMat(bvp->iga,&A);CHKERRQ(ierr);
  ierr = IGACreateVec(bvp->iga,&b);CHKERRQ(ierr);
  ierr = MatSetOption(A,MAT_SYMMETRIC,PETSC_TRUE);CHKERRQ(ierr);
  ierr = IGASetFormFunction(bvp->iga,FunctionR,bvp);CHKERRQ(ierr);
  ierr = IGASetFormJacobian(bvp->iga,JacobianL2,bvp);CHKERRQ(ierr);
  ierr = IGAComputeFunction(bvp->iga,U,b);CHKERRQ(ierr);
  ierr = IGAComputeJacobian(bvp->iga,U,A);CHKERRQ(ierr);
  //Solver
  {
    KSP ksp;
    ierr = IGACreateKSP(bvp->iga,&ksp);CHKERRQ(ierr);
    ierr = KSPSetOperators(ksp,A,A);CHKERRQ(ierr);
    ierr = KSPSetFromOptions(ksp);CHKERRQ(ierr);
    ierr = KSPSolve(ksp,b,bvp->xDirichlet);CHKERRQ(ierr);
    ierr = KSPDestroy(&ksp);CHKERRQ(ierr);
  }
  
  char           filename[256];
  sprintf(filename,"./reaction%d.vts",bvp->load_increment);
  ierr = IGADrawVecVTK(bvp->iga,bvp->xDirichlet,filename);CHKERRQ(ierr);
  
  ierr = VecDestroy(&b);CHKERRQ(ierr);
  ierr = MatDestroy(&A);CHKERRQ(ierr);
  
  PetscFunctionReturn(0);
}


#endif
