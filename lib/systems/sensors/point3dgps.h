#ifndef GCOP_POINT3DGPS_H
#define GCOP_POINT3DGPS_H

#include <Eigen/Dense>
#include "sensor.h"
#include "point3d.h"

namespace gcop {

	using namespace Eigen;

	//  typedef Point3dgps<6> FullPoint3dgps;
	//  typedef Point3dgps<3> AccPoint3dgps;

	/**
	 * General sensor model 
	 *
	 * Subclasses should provide implementation for the 
	 * sensor function ()
	 *
	 * Author: Marin Kobilarov marin(at)jhu.edu
	 */
  template<int _nu = 3, int _np = Dynamic>
	class Point3dGps : public Sensor<Point3dState, 6, _nu, _np, Vector3d, 3> {

		public:  

      typedef Matrix<double, _nu, 1> Vectorcd;
      typedef Matrix<double, _np, 1> Vectormd;

			typedef Matrix<double, 3, 6> Matrix36d;
      typedef Matrix<double, 3, _nu> Matrixrcd;
      typedef Matrix<double, 3, _np> Matrixrmd;

			Point3dGps(): Sensor<Point3dState, 6, _nu, _np, Vector3d, 3>(Rn<3>::Instance()) {    

				sxy = .02;
				sz = .02;

				this->R(0,0) = sxy*sxy;
				this->R(1,1) = sxy*sxy;
				this->R(2,2) = sz*sz;
			}


			bool operator()(Vector3d &y, double t, const Point3dState &x, const Vectorcd &u,
					const Vectormd *p = 0, 
					Matrix36d *dydx = 0, Matrixrcd *dydu = 0,
					Matrixrmd *dydp = 0) {

				y = x.q; //#TODO Sample the sensor output from a gaussian distribution of x.q and this->R

				if (dydx){
					dydx->topLeftCorner<3,3>().setIdentity();
				}

				return true;    
			}

			double sxy;   ///< x-y position standard deviation
			double sz;    ///< altitute stdev
	};


	// Point3dGps::Point3dGps() : 
}

#endif
