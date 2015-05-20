/*********************************************************************
* Software License Agreement (BSD License)
* 
*  Copyright (c) 2008, Willow Garage, Inc.
*  All rights reserved.
* 
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
* 
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of the Willow Garage nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
* 
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*********************************************************************/

/* Author: Wim Meeussen */

#ifndef URDF_PARSER_URDF_PARSER_H
#define URDF_PARSER_URDF_PARSER_H

#include <string>
#include <map>
#include <tinyxml.h>
#include <boost/function.hpp>
#include "urdfmodel.h"
#include "mbs.h"
#include "airbase.h"


namespace gcop_urdf{

	boost::shared_ptr<ModelInterface> parseURDF(const std::string &xml_string);
	void transformtoprincipal(boost::shared_ptr<Link> link);
	void combineinertia(boost::shared_ptr<Link> clink, boost::shared_ptr<Link> plink,Pose posec_p);
	void assign(boost::shared_ptr<const Link> link, boost::shared_ptr<Link> parentlink,Pose cumpose);
	void aggregate(boost::shared_ptr<const Link> link, boost::shared_ptr<Link> parentlink,Pose cumpose);
	gcop::Matrix4d diffpose(Pose &posej_p,Pose &posei_p);
	void walkTree(boost::shared_ptr<Link> link, int level,int &index,boost::shared_ptr<gcop::Mbs> mbs);
  /**
  * Creates a Multi body system from urdf
  * 
  * np is the number of parameters
  */
	boost::shared_ptr<gcop::Mbs> mbsgenerator(const std::string &xml_string,gcop::Matrix4d &gposei_root, std::string type = "FLOATBASE", int np = 0);

}

#endif
