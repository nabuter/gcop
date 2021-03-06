#include <iomanip>
#include <iostream>
#include "viewer.h"
#include "body3dview.h"
//#include "body3dcost.h"
#include "lqsensorcost.h"
#include "utils.h"
#include "so3.h"
#include "params.h"
#include "sensor.h"
#include "insmanifold.h"

//#define USE_STOCHASTIC_DYNAMICS

#ifdef USE_STOCHASTIC_DYNAMICS
#include "gndoep.h"
#else
#include "deterministicgndoep.h"
#endif

using namespace std;
using namespace Eigen;
using namespace gcop;

Params params;


void projectmanifold(const Body3dState &bodystate, InsState &pstate)
{
  pstate.R = bodystate.R;
  pstate.p = bodystate.p;;
  
  pstate.v = bodystate.v;
    
  //bg, ba do not care 
}

void solver_process(Viewer* viewer)
{
  if (viewer)
    viewer->SetCamera(-2.5, 81, -1.8, -2.15, -6.3);

  int iters = 3;
  params.GetInt("iters", iters);

  int N = 20;
  params.GetInt("N", N);

  double tf = 2;
  params.GetDouble("tf", tf);

  double h = tf/N;
  
  Body3d<> sys(6);//Number of Parameters

  //Add sensor
  Sensor<InsState, 15, 6> imugps(InsManifold::Instance());//Create a default sensor which just copies the InsManifold over

  Body3dState xf;
  xf.Clear();

  //  Body3dCost<> cost(tf, xf);
  //<Tx, nx, nu, nres, np, Tz, nz>
  LqSensorCost<Body3dState, 12, 6, Dynamic, Dynamic, InsState, 15> cost(sys, imugps.Z);

  VectorXd R(15);//Z
  if (params.GetVectorXd("R", R))
    cost.R = R.asDiagonal();

  VectorXd S(12);//X
  if (params.GetVectorXd("S", S))
    cost.S = S.asDiagonal();
  
  VectorXd P(6);//Parameters trying to estimate (External forces)
  if (params.GetVectorXd("P", P)) 
    cost.P = P.asDiagonal();

  cost.UpdateGains();

  // times

  vector<double> ts(N+1);
  for (int k = 0; k <=N; ++k)
    ts[k] = k*h;

  // states
  vector<Body3dState> xs(N+1);
  Vector3d e0(1.2, -2, 1);
  SO3::Instance().exp(xs[0].R, e0);
  xs[0].p <<  5, 5, 5;
  xs[0].w.setZero();
  xs[0].v.setZero();

  // controls 
  vector<Vector6d> us(N);
  for (int i = 0; i < N/2; ++i) {
    for (int j = 0; j < 3; ++j) {
      us[i][j] = .0;
    }
    for (int j = 3; j < 6; ++j) {
      us[i][j] = 0;
    }
    us[N/2+i] = -us[i];
  }

  //Sensor
  vector<InsState> zs(N/2);//Same as ts_sensor
  vector<double> ts_sensor(N/2);

  //Set sensor times:
  for (int k = 0; k <(N/2); ++k)
    ts_sensor[k] = 2*k*h;

  VectorXd mup(6);//Initial Prior
  VectorXd p0(6);//Initial Guess for external forces

  p0<<0,0,0,0,0,-0.05;//Initialguess
  if (!params.GetVectorXd("p0", p0)) 
  {
    cout<<"Cannot find p0 initial guess for parameters"<<endl;
  }
  mup = p0;//Copy the initial guess to be the same as prior for the parameters

  VectorXd pd(6);///<True External forces
  pd<<0,0,0.05,0,0.1,-0.1;
  params.GetVectorXd("pd", pd); 
  //pd<<0,0,0.05,0,0.0,0.0;

  
  /////Creating the Optimization problem////

  //Temporary point3d state:
  InsState projected_state;
  int sensor_index = 0;

  //First evaluate true trajectory and thus sensor values associated with it:
  //Initialize random seed for adding noise to sensor data:
  srand(10281048);//Random number
  Matrix3d rotationmatrix_rand;
  Vector3d rotation_randvec;
  sys.Reset(xs[0], ts[0]);
  for (int i = 0; i < N; ++i) {
    //Can also add noise here for system
    sys.Step(xs[i+1], ts[i], xs[i], us[i], h, &pd);//Step the system
    //Update sensor based on sensor timings
    //Can also add noise here for sensor measurment
    if((ts_sensor[sensor_index] - ts[i])>= 0 && (ts_sensor[sensor_index] - ts[i+1]) < 0)
    {
      int near_index = (ts_sensor[sensor_index] - ts[i]) > -(ts_sensor[sensor_index] - ts[i+1])?(i+1):i;
      //Project the state
      projectmanifold(xs[near_index],projected_state);
      //cout<<"Projected state: "<<projected_state.q.transpose()<<endl;
      imugps(zs[sensor_index], ts[near_index], projected_state, us[near_index]);
      //Add noise:
      rotation_randvec.setZero();
      for(int count = 0; count < 3; count++)
      {
        assert(R[count] >= 0.001);
        rotation_randvec[count] += sqrt(1/R[count])*randn();//Util function  

        assert(R[count+9] >= 0.001);
        zs[sensor_index].p[count] += sqrt(1/R[count+9])*randn();//Util function  

        assert(R[count+12] >= 0.001);
        zs[sensor_index].v[count] += sqrt(1/R[count+12])*randn();//Util function  
      }
      SO3::Instance().exp(rotationmatrix_rand, rotation_randvec);
      zs[sensor_index].R *= rotationmatrix_rand;
      cout<<"Zs ["<<(sensor_index)<<"]: "<<zs[sensor_index].p.transpose()<<"\t"<<endl<<zs[sensor_index].R<<endl<<"\tts_sensor: "<<ts_sensor[sensor_index]<<endl;
      sensor_index = sensor_index < (ts_sensor.size()-1)?sensor_index+1:sensor_index;
    }
  }

  Body3dView<> view(sys, &xs);
  viewer->Add(view);  

  getchar();

  //Assign  Zs and prior for parameters:
  cost.SetReference(&zs, &mup);//Set reference for zs

  //Create Gauss newton estimation problem with initial guess for parameters p0
  //<Tx, nx, nu, np, nres, Tz, nz, T1=T, nx1=nx>
  GnDoep<Body3dState, 12, 6, Dynamic, Dynamic, InsState, 15, InsState, 15> gn(sys, imugps, cost, ts, xs, us, p0, ts_sensor, &projectmanifold);  
  getchar();


  ////SOLVE OPTIMIZATION PROBLEM////
  gn.debug = false; // turn off debug for speed

  struct timeval timer;

  for (int i = 0; i < iters; ++i) {
    timer_start(timer);
    gn.Iterate();
    long te = timer_us(timer);
    cout<<"Cost: "<<(gn.J)<<endl;
    cout << "Parameter: "<< p0 << endl;
    cout << "Iteration #" << i << " took: " << te << " us." << endl;
    getchar();
  }
  
  cout << "done!" << endl;
  while(1)
    usleep(10);    
}


#define DISP

int main(int argc, char** argv)
{

  if (argc > 1)
    params.Load(argv[1]);
  else
    params.Load("../../bin/body3dforceest.cfg"); 

#ifdef DISP
  Viewer *viewer = new Viewer;
  viewer->Init(&argc, argv);
  viewer->frameName = "body3d/frames/frame";

  pthread_t dummy;
  pthread_create( &dummy, NULL, (void *(*) (void *)) solver_process, viewer);

#else
  solver_process(0);
#endif


#ifdef DISP
  viewer->Start();
#endif


  return 0;
}
