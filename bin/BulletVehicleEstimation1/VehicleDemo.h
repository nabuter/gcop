/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2006 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/
#ifndef VEHICLE_DEMO_H
#define VEHICLE_DEMO_H

class btVehicleTuning;
struct btVehicleRaycaster;
class btCollisionShape;

#include "BulletDynamics/Vehicle/btRaycastVehicle.h"

#include "GlutDemoApplication.h"

#include <stdio.h>

#include <stdlib.h>

#include "point3dgps.h"
#include "gndoep.h"
#include "params.h"
#include "lqsensorcost.h"
#include "utils.h"
#include "rccar.h"

using namespace gcop;
using namespace std;
using namespace Eigen;

///VehicleDemo shows how to setup and use the built-in raycast vehicle
class VehicleDemo : public GlutDemoApplication
{
  typedef LqSensorCost<Vector4d, 4, 2, Dynamic, 9, Vector3d, 3> RccarCost;
  typedef GnDoep<Vector4d, 4, 2, Dynamic, 9, Vector3d, 3, Point3dState, 6> RccarDoep;

  protected:
    int iters; //Number of iterations
    double tf;    // time horizon
    int N;// Number of segments
    VectorXd p0;//Initial Guess for parameters
    Point3dGps<2> gps;//Gps sensor with controls and nofparams
    Rccar sys;
    RccarCost *cost;
    RccarDoep *gn;
    vector<Vector4d> xs;
    vector<Vector3d> zs;
    vector<Vector2d> us;
    vector<double> ts;
    vector<double> ts_sensor;

	public:

	btRigidBody* m_carChassis;

	btAlignedObjectArray<btCollisionShape*> m_collisionShapes;

	class btBroadphaseInterface*	m_overlappingPairCache;

	class btCollisionDispatcher*	m_dispatcher;

	class btConstraintSolver*	m_constraintSolver;

	class btDefaultCollisionConfiguration* m_collisionConfiguration;

	class btTriangleIndexVertexArray*	m_indexVertexArrays;

	btVector3*	m_vertices;

	
	btRaycastVehicle::btVehicleTuning	m_tuning;
	btVehicleRaycaster*	m_vehicleRayCaster;
	btRaycastVehicle*	m_vehicle;
	btCollisionShape*	m_wheelShape;

	float		m_cameraHeight;

	float	m_minCameraDistance;
	float	m_maxCameraDistance;


	VehicleDemo();

	virtual ~VehicleDemo();

	virtual void clientMoveAndDisplay();

  void renderTrajectory(vector<Vector3d> *zs, btVector3 *color);

	virtual void	clientResetScene();

	virtual void displayCallback();
	
	///a very basic camera following the vehicle
	virtual void updateCamera();

	virtual void specialKeyboard(int key, int x, int y);

	virtual void specialKeyboardUp(int key, int x, int y);

  void keyboardCallback(unsigned char key, int x, int y);

	void renderme();

	void initPhysics();

  void sampleSensor(Vector3d &z);

	static DemoApplication* Create()
	{
		VehicleDemo* demo = new VehicleDemo();
		demo->myinit();
		demo->initPhysics();
		return demo;
	}

  static void projectmanifold(const Vector4d &rccarstate, Point3dState &pstate)
  {
    pstate.q.head<2>() = rccarstate.head<2>();//First 2 elements are x and y
    pstate.q[2] = 0;//Set z to 0. This is enough for GPS
  }
};

#endif //VEHICLE_DEMO_H


