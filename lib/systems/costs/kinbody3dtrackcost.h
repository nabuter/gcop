#ifndef GCOP_KINBODY3DTRACKCOST_H
#define GCOP_KINBODY3DTRACKCOST_H

#include <limits>
#include <iostream>
#include "so3.h"
#include "cost.h"
#include "kinbody3dtrack.h"
#include "kinbody3d.h"

namespace gcop {
  
  using namespace std;
  using namespace Eigen;

  typedef Matrix<double, Dynamic, 6> MatrixX6d;

  template <int _nu = 6>
  class Kinbody3dTrackCost : public Cost<Matrix4d, 6, _nu> {

  public:
    typedef Matrix<double, _nu, _nu> Matrixud;
    typedef Matrix<double, _nu, 1> Vectorud;
    typedef Matrix<double, 6, 1> Vector6d;
    typedef Matrix<double, 6, 6> Matrix6d;
    
    Kinbody3dTrackCost(double tf, const Kinbody3dTrack<_nu> &pg);
    
    double L(double t, const Matrix4d &x, const Vectorud &u, double h,
             const VectorXd *p,
             Vector6d *Lx = 0, Matrix6d *Lxx = 0,
             Vectorud *Lu = 0, Matrixud *Luu = 0,
             Matrix<double, 6, _nu> *Lxu = 0, 
             VectorXd *Lp = 0, MatrixXd *Lpp = 0, MatrixX6d *Lpx = 0);


    const Kinbody3dTrack<_nu> &pg;
  };  

template <int _nu> 
Kinbody3dTrackCost<_nu>::Kinbody3dTrackCost(double tf, const Kinbody3dTrack<_nu> &pg) :
  Cost<Matrix4d, 6, _nu>(pg.sys, tf), pg(pg)
{
  
}


static Matrix3d r3hat(const Vector3d &a)
{
  SO3& so3 = SO3::Instance();
  Matrix3d a_hat;

  so3.hat(a_hat, a);
  //a_hat << 0,  -a(2),  a(1),
  //       a(2),    0,  -a(0),
  //      -a(1),  a(0),    0;
  return a_hat;
}


static Vector3d cross3(Vector3d a, Vector3d b)
{
  return a.cross(b);
}

template <int _nu> 
double Kinbody3dTrackCost<_nu>::L(double t, const Matrix4d &x, const Vectorud &u, 
                          double h,
                          const VectorXd *p,
                          Vector6d *Lx, Matrix6d *Lxx,
                          Vectorud *Lu, Matrixud *Luu,
                          Matrix<double, 6, _nu> *Lxu,
                          VectorXd *Lp, MatrixXd *Lpp, MatrixX6d *Lpx)
{
  double L = 0;

  //cout << "x: " << endl << x << endl;

  if (Lu)
    Lu->setZero();

  if (Luu)
    Luu->setZero();

  if (Lxu)
    Lxu->setZero();
  
  if (Lx)
    Lx->setZero();

  if (Lxx)
    Lxx->setZero();

  if (Lp)
    Lp->setZero();

  if (Lpp)
    Lpp->setZero();

  if (Lpx)
    Lpx->setZero();
  
  //const Matrix3d &g = x.first;

  const Matrix3d &R = x.block<3,3>(0,0); // orientation
  const Vector3d &xp = x.block<3,1>(0,3);     // position
  
  int N = pg.Is.size() - 1;
  
  h = this->tf/N;

  //  int k = (h < 1e-16 ? N : (int)round(t/h));

  int k = (int)round(t/h);
  assert(k >=0 && k <= N);
  const vector< pair<int, Vector3d> > &I = pg.Is[k];

  //cout << "Kinbody3dtrackCost: k=" << k << " " << I.size() << endl;

  int i0 = 3*pg.extforce;
  for (int i = 0; i < I.size(); ++i) {
    int l = pg.cis[I[i].first];  // feature index
    const Vector3d &z = I[i].second;

    const Vector3d &pf = p->segment<3>(i0 + 3*l);
    Vector3d y = R.transpose()*(pf - xp);
    Vector3d r = (y - z)/pg.cp;

    L += r.dot(y - z)/2;         // feature cost

    //cout << "feature cost " << i << "=" << r.dot(y - z)/2 << endl;

    if (Lx) {
      //Lx->segment<3>(0) = Lx->segment<3>(0) - cross3(y, r);
      Lx->segment<3>(0) = Lx->segment<3>(0) - r3hat(y)*r;
      Lx->segment<3>(3) = Lx->segment<3>(3) - r;
    }

    if (Lxx) {
      Lxx->block<3,3>(0,0) += (r3hat(y).transpose()*r3hat(y))/pg.cp + r3hat(r)*r3hat(y)/2. + (r3hat(r)*r3hat(y)).transpose()/2.;
      Lxx->block<3,3>(0,3) += (-r3hat(r)/2. + r3hat(y)/pg.cp);
      Lxx->block<3,3>(3,0) += (-r3hat(r)/2. + r3hat(y)/pg.cp).transpose();      
      Lxx->block<3,3>(3,3) += Matrix3d::Identity()/pg.cp;
    }

    if (Lp)
      Lp->segment<3>(i0 + 3*l) = R*r;

    if (Lpp)
    {
      Lpp->block<3,3>(i0 + 3*l, i0 + 3*l) = Matrix3d::Identity()/pg.cp;
      //Lpp->block<3,3>(i0 + 3*l, i0 + 3*l) = Matrix3d::Identity();
    }

    if (Lpx) {
      Lpx->block<3,3>(i0 + 3*l, 0) = R*((r3hat(y)/pg.cp) - r3hat(r));
      Lpx->block<3,3>(i0 + 3*l, 3) = -R/pg.cp;
      //Lpx->block<3,3>(i0 + 3*l, 3) = Eigen::MatrixXd::Zero(3,3);
    }
  }


  if (pg.forces && k < N) {
    Vectorud du = u - pg.uos[k];
    Vectorud wdu = du.cwiseQuotient(pg.cw);

    L += du.dot(wdu)/2;
    if (Lu)
      *Lu = wdu;
    if (Luu)
      *Luu = Vectorud::Ones().cwiseQuotient(pg.cw).asDiagonal();
  }

  /*
  if(Lp)
    cout << "Lp: " << endl << *Lp << endl;
  if(Lpp)
    cout << "Lpp: " << endl << *Lpp << endl;
  if(Lpx)
    cout << "Lpx: " << endl << *Lpx << endl;
  if(Lx)
    cout << "Lx: " << endl << *Lx << endl;
  if(Lxx)
    cout << "Lxx: " << endl << *Lxx << endl;
  */

  return L;
}
}

#endif
