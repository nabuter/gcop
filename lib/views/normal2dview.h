#ifndef GCOP_NORMAL2DVIEW_H
#define GCOP_NORMAL2DVIEW_H

#include "normal.h"
#include "systemview.h"

namespace gcop {
  using namespace Eigen;

  template<int _n>
  class Normal2dView : public SystemView<Normal<_n>, Vector2d> {
  public:
    /**
     *  Create a normal view of trajectory traj
     * @param sys normal
     * @param xs trajectory
     */
    Normal2dView(vector<Normal<_n> > *gds = 0,
                 vector<Vector2d> *us = 0);

    virtual ~Normal2dView();       

    void Render(const Normal<_n> *gd,
                const Vector2d *u = 0);
    
    void Render(const vector<Normal<_n> > *gds = 0,
                const vector<Vector2d> *us = 0,
                bool rs = true,
                int is = -1, int ie = -1,
                int dis = 1, int dit = 1,
                bool dl = false);

    bool wire;
    bool texture;
    double s; ///< height scaling (default is 1)
  };

template<int _n>
Normal2dView<_n>::Normal2dView(vector<Normal<_n> > *xs, vector<Vector2d> *us) : 
  SystemView("Normal", xs, us)
{
  rgba[0] = 0.5;
  rgba[1] = 0.5;
  rgba[2] = 0.5;
  rgba[3] = 0;
  this->lineWidth = 2;

  wire= false;
  texture = false;
  s = 1;
}


template<int _n>
Normal2dView<_n>::~Normal2dView()
{
}


template<int _n>
void Normal2dView<_n>::Render(const Normal<_n> *gd, const Vector2d *u)
{
  glColor4dv(rgba);
  
  if (texture) {
    glColor4f(1,1,1,1);  
    glDisable(GL_LIGHTING);
  } else {
    glEnable(GL_LIGHTING);
  }

  double c = .5;
  Viewer::SetMaterial(c,c,c, c,c,c, c,c,c,5);
  //  glColor3f(.5,.5,.5);

  if (wire) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //    glDisable(GL_DEPTH_TEST);
    //    glCullFace(GL_BACK);
  }

  double xl = gd->mu[0] - 2*sqrt(gd->P(0,0));
  double xu = gd->mu[0] + 2*sqrt(gd->P(0,0));
  double yl = gd->mu[1] - 2*sqrt(gd->P(1,1));
  double yu = gd->mu[1] + 2*sqrt(gd->P(1,1));
  int N = 50;
  double dx = (xu - xl)/N;
  double dy = (yu - yl)/N;
  
  for (double y = yl; y < yu; y+=dy) {
    //Makes OpenGL draw a triangle at every three consecutive vertices
    glBegin(GL_TRIANGLE_STRIP);    
    for (double x = xl; x < xu; x+=dx) {

      double l = s*gd->L(Vector2d(x, y));
      
      //       glNormal3dv(dem.normals + 3*(i*dem.nj+j));

      glVertex3f(x, y, l);
      //      if (texture)
      //        glTexCoord2d( (p[0]-dem.o[0])/dem.w, (dem.h - (p[1]-dem.o[1]) )/dem.h);
      
      l = s*gd->L(Vector2d(x, y+dy));
      //      glNormal3dv(dem.normals + 3*((i+1)*dem.nj+j));
      
      glVertex3f(x, y + dy, l);
      //      if (texture)
      //        glTexCoord2d( (p[0]-dem.o[0])/dem.w, (dem.h - (p[1]-dem.o[1]) )/dem.h);      
    }
    glEnd();
  }

  if (wire)
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


  if (texture)
    glEnable(GL_LIGHTING);
}


void Normal2dView::Render(const vector<Normal> *gds, 
                          const vector<Vector2d> *us,
                          bool rs, 
                          int is, int ie,
                          int dis, int dit,
                          bool dl)
{
  Viewer::SetColor(rgba[0], rgba[1], rgba[2], rgba[3]);
  //  glColor4f(

  // set defaults
  if (is == -1)
    is = 0;
  if (ie == -1)
    ie = gds->size()-1;

  assert(is >= 0 && is <= gds->size()-1 && ie >= 0 && ie <= gds->size()-1);
  assert(is <= ie);

  glDisable(GL_LIGHTING);
  glLineWidth(lineWidth);
  glBegin(GL_LINE_STRIP);
  for (int i = is; i <= ie; i+=dit) {
    glVertex3d((*gds)[i].mu[0], (*gds)[i].mu[1], 0);
  }
  glEnd();
  glLineWidth(1);
  glEnable(GL_LIGHTING);
  
  if (rs) {
    for (int i = 0; i < gds->size(); i+=dis) {
      Render(&(*gds)[i]);
    }
  }

  if (dl)
    Render(&gds->back());
}

}

#endif
