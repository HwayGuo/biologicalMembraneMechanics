/*
  Kirchhoff-Love shell kinematics 
  author: Shiva Rudraraju
*/
#if !defined(KINEMATICS_H_)
#define KINEMATICS_H_

template <class T>
struct  KinematicsStruct{
  //coordinates in current configuration
  double x0[3]; 
  //gradient and Hessian maps
  double dXdR[3][2];
  double dXdR2[3][2][2];
  T dxdR[3][2];
  T dxdR2[3][2][2];
  T dxdR_contra[3][2];
  //kinematic quantities
  double A[2][2]; T a[2][2];
  double A_contra[2][2]; T a_contra[2][2];
  double B[2][2]; T b[2][2];
  double B_contra[2][2]; T b_contra[2][2];
  T Gamma[2][2][2];
  //
  T J, J_A, J_a;
  T normal[3]; double Normal[3];
  //
  T I1;
  double H0;
  T H, Kappa;
  T q; //Lagrange multiplier
  //stabilization related quatities
  double dx0dR[3][2], dx0dR2[3][2][2];
  double aPre[2][2], aPre_contra[2][2];
  T JPre, I1Pre;
};


#undef  __FUNCT__
#define __FUNCT__ "getKinematics"
template <class T>
PetscErrorCode getKinematics(IGAPoint p, const T * tempU, const PetscScalar * tempU0, KinematicsStruct<T>& k){
  PetscFunctionBegin;

  //get number of shape functions (nen) and dof's
  PetscInt nen, dof;
  IGAPointGetSizes(p,0,&nen,&dof);

  //shape functions: value, grad, hess
  const PetscReal (*N) = (const PetscReal (*)) p->shape[0];
  const PetscReal (*N1)[2] = (const PetscReal (*)[2]) p->basis[1];
  const PetscReal (*N2)[2][2] = (const PetscReal (*)[2][2]) p->basis[2];
  
  //get X
  const PetscReal (*X)[3] = (const PetscReal (*)[3]) p->geometry;

  //get x, q
  T x[nen][3]; double x0[nen][3];
#ifdef LagrangeMultiplierMethod
  T (*u)[3+1] = (T (*)[3+1])tempU;
  double (*u0)[3+1] = (double (*)[3+1])tempU0;
  T q=0.0; //q is the Lagrange multiplier value at this point
#else
  T (*u)[3] = (T (*)[3])tempU;
  double (*u0)[3] = (double (*)[3])tempU0;
#endif
  for(unsigned int n=0; n<(unsigned int) nen; n++){
    for(unsigned int d=0; d<3; d++){
      x[n][d]= X[n][d]+ u[n][d];
      x0[n][d]= X[n][d]+ u0[n][d];
    }
#ifdef LagrangeMultiplierMethod
    q+=N[n]*u[n][3];
#endif
  }
#ifdef LagrangeMultiplierMethod
  k.q=q;
#endif

  //get x0
  for(unsigned int d=0; d<3; d++){
    k.x0[d]=0.0;
    for(unsigned int n=0; n<(unsigned int) nen; n++){
      k.x0[d]+=N[n]*(X[n][d]+ u0[n][d]);
    }
  }

  //basis vectors, dXdR and dxdR, gradient of basis vectors, dXdR2 and dxdR2
  for(unsigned int d=0; d<3; d++){
    k.dXdR[d][0]=k.dXdR[d][1]=0.0;
    k.dXdR2[d][0][0]=k.dXdR2[d][0][1]=0.0;
    k.dXdR2[d][1][0]=k.dXdR2[d][1][1]=0.0;
    k.dxdR[d][0]=k.dxdR[d][1]=0.0;
    k.dxdR2[d][0][0]=k.dxdR2[d][0][1]=0.0;
    k.dxdR2[d][1][0]=k.dxdR2[d][1][1]=0.0;
    k.dx0dR[d][0]=k.dx0dR[d][1]=0.0;
    k.dx0dR2[d][0][0]=k.dx0dR2[d][0][1]=0.0;
    k.dx0dR2[d][1][0]=k.dx0dR2[d][1][1]=0.0;
    for(unsigned int n=0; n<(unsigned int) nen; n++){
      k.dXdR[d][0]+=N1[n][0]*X[n][d];
      k.dXdR[d][1]+=N1[n][1]*X[n][d];
      k.dXdR2[d][0][0]+=N2[n][0][0]*X[n][d];
      k.dXdR2[d][0][1]+=N2[n][0][1]*X[n][d];
      k.dXdR2[d][1][0]+=N2[n][1][0]*X[n][d];	    
      k.dXdR2[d][1][1]+=N2[n][1][1]*X[n][d];
      //
      k.dxdR[d][0]+=N1[n][0]*x[n][d];
      k.dxdR[d][1]+=N1[n][1]*x[n][d];
      k.dxdR2[d][0][0]+=N2[n][0][0]*x[n][d];
      k.dxdR2[d][0][1]+=N2[n][0][1]*x[n][d];
      k.dxdR2[d][1][0]+=N2[n][1][0]*x[n][d];	    
      k.dxdR2[d][1][1]+=N2[n][1][1]*x[n][d];
      //
      k.dx0dR[d][0]+=N1[n][0]*x0[n][d];
      k.dx0dR[d][1]+=N1[n][1]*x0[n][d];
      k.dx0dR2[d][0][0]+=N2[n][0][0]*x0[n][d];
      k.dx0dR2[d][0][1]+=N2[n][0][1]*x0[n][d];
      k.dx0dR2[d][1][0]+=N2[n][1][0]*x0[n][d];	    
      k.dx0dR2[d][1][1]+=N2[n][1][1]*x0[n][d];
    }
  }

  //metric tensors A, a, aPre
  for(unsigned int i=0; i<2; i++){
    for(unsigned int j=0; j<2; j++){
      k.A[i][j]=0.0;
      k.a[i][j]=0.0;
      k.aPre[i][j]=0.0;
      for(unsigned int d=0; d<3; d++){
	k.A[i][j]+=k.dXdR[d][i]*k.dXdR[d][j];
	k.a[i][j]+=k.dxdR[d][i]*k.dxdR[d][j];
	k.aPre[i][j]+=k.dx0dR[d][i]*k.dx0dR[d][j];
      }
    }
  }

  //Jacobians
  T J_a; double J_A, J_aPre;
  J_A=std::sqrt(k.A[0][0]*k.A[1][1]-k.A[0][1]*k.A[1][0]);
  J_a=std::sqrt(k.a[0][0]*k.a[1][1]-k.a[0][1]*k.a[1][0]);
  J_aPre=std::sqrt(k.aPre[0][0]*k.aPre[1][1]-k.aPre[0][1]*k.aPre[1][0]);
  if (J_A<=0.0) {std::cout << "negative jacobian\n";  exit(-1);}
  k.J_A=J_A; k.J_a=J_a; 
  k.J=J_a/J_A; 
  k.JPre=J_a/J_aPre;

  //surface normals
  k.normal[0]=(k.dxdR[1][0]*k.dxdR[2][1]-k.dxdR[2][0]*k.dxdR[1][1])/J_a;
  k.normal[1]=(k.dxdR[2][0]*k.dxdR[0][1]-k.dxdR[0][0]*k.dxdR[2][1])/J_a;
  k.normal[2]=(k.dxdR[0][0]*k.dxdR[1][1]-k.dxdR[1][0]*k.dxdR[0][1])/J_a;
  k.Normal[0]=(k.dXdR[1][0]*k.dXdR[2][1]-k.dXdR[2][0]*k.dXdR[1][1])/J_A;
  k.Normal[1]=(k.dXdR[2][0]*k.dXdR[0][1]-k.dXdR[0][0]*k.dXdR[2][1])/J_A;
  k.Normal[2]=(k.dXdR[0][0]*k.dXdR[1][1]-k.dXdR[1][0]*k.dXdR[0][1])/J_A;

  //curvature tensors B, b
  for(unsigned int i=0; i<2; i++){
    for(unsigned int j=0; j<2; j++){
      k.b[i][j]=0.0; k.B[i][j]=0.0;
      for(unsigned int d=0; d<3; d++){
	k.b[i][j]+=k.normal[d]*k.dxdR2[d][i][j];
	k.B[i][j]+=k.Normal[d]*k.dXdR2[d][i][j];
      }
    }
  }

  //determinants of metric tensors and curvature tensor
  T det_a=k.a[0][0]*k.a[1][1]-k.a[0][1]*k.a[1][0];
  T det_b=k.b[0][0]*k.b[1][1]-k.b[0][1]*k.b[1][0];
  double det_A=k.A[0][0]*k.A[1][1]-k.A[0][1]*k.A[1][0];
  double det_aPre=k.aPre[0][0]*k.aPre[1][1]-k.aPre[0][1]*k.aPre[1][0];
  
  //contra-variant metric tensors, a_contra, A_contra, aPre_contra.
  k.a_contra[0][0]=k.a[1][1]/det_a; k.a_contra[1][1]=k.a[0][0]/det_a;
  k.a_contra[0][1]=-k.a[0][1]/det_a; k.a_contra[1][0]=-k.a[1][0]/det_a;
  k.A_contra[0][0]=k.A[1][1]/det_A; k.A_contra[1][1]=k.A[0][0]/det_A;
  k.A_contra[0][1]=-k.A[0][1]/det_A; k.A_contra[1][0]=-k.A[1][0]/det_A;
  k.aPre_contra[0][0]=k.aPre[1][1]/det_aPre; k.aPre_contra[1][1]=k.aPre[0][0]/det_aPre;
  k.aPre_contra[0][1]=-k.aPre[0][1]/det_aPre; k.aPre_contra[1][0]=-k.aPre[1][0]/det_aPre;
  for(unsigned int d=0; d<3; d++){
    k.dxdR_contra[d][0]=k.a_contra[0][0]*k.dxdR[d][0]+k.a_contra[0][1]*k.dxdR[d][1];
    k.dxdR_contra[d][1]=k.a_contra[1][0]*k.dxdR[d][0]+k.a_contra[1][1]*k.dxdR[d][1];
  }

  //contra-variant curvature tensors B_contra, b_contra.
  for(unsigned int i=0; i<2; i++){
    for(unsigned int j=0; j<2; j++){
      k.B_contra[i][j]=0.0;
      k.b_contra[i][j]=0.0;
      for(unsigned int z=0; z<2; z++){
	for(unsigned int l=0; l<2; l++){
	  k.B_contra[i][j]+=k.A_contra[i][z]*k.B[z][l]*k.A_contra[l][j]; //*
	  k.b_contra[i][j]+=k.a_contra[i][z]*k.b[z][l]*k.a_contra[l][j]; //*
	}
      }
    }
  }
  
  //Christoffel symbols
  for(unsigned int i=0; i<2; i++){
    for(unsigned int j=0; j<2; j++){
      for(unsigned int z=0; z<2; z++){
	k.Gamma[i][j][z]=0.0;
	for(unsigned int d=0; d<3; d++){
	  k.Gamma[i][j][z]+=k.dxdR_contra[d][i]*k.dxdR2[d][j][z];
	}
      }
    }
  }

  //invariants of the Green-Lagrange strain, C
  T I1=0.0, I1Pre=0.0;
  for(unsigned int i=0; i<2; i++){
    for(unsigned int j=0; j<2; j++){
      I1+=k.A_contra[i][j]*k.a[i][j];
      I1Pre+=k.aPre_contra[i][j]*k.a[i][j];
    }
  }
  k.I1=I1;
  k.I1Pre=I1Pre;
  
  //mean curvature: H
  T H=0; PetscReal H0=0.0;
  for (unsigned int i=0; i<2; i++){
    for (unsigned int j=0; j<2; j++){
      H+=0.5*k.a[i][j]*k.b_contra[i][j];  //current curvature
      H0+=0.5*k.A[i][j]*k.B_contra[i][j]; //reference curvature
    }
  }
  k.H=H; k.H0=H0;

  //Gaussian curvature: Kappa
  k.Kappa=det_b/det_a;
  
  PetscFunctionReturn(0);
}

#endif
