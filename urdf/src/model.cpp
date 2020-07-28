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

#include "urdf/model.h"
#include <urdf_parser_plugin/parser.h>
#include <pluginlib/class_loader.hpp>

#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <utility>
#include <vector>


namespace urdf
{
class ModelImplementation
{
public:
  ModelImplementation()
  : loader_("urdf_parser_plugin", "urdf::URDFParser")
  {
  }

  ~ModelImplementation() = default;

  // Loader used to get plugins
  pluginlib::ClassLoader<urdf::URDFParser> loader_;
};


Model::Model()
: impl_(new ModelImplementation)
{
}

Model::~Model()
{
  clear();
  impl_.reset();
}


bool Model::initFile(const std::string & filename)
{
  // get the entire file
  std::string xml_string;
  std::fstream xml_file(filename.c_str(), std::fstream::in);
  if (xml_file.is_open()) {
    while (xml_file.good() ) {
      std::string line;
      std::getline(xml_file, line);
      xml_string += (line + "\n");
    }
    xml_file.close();
    return Model::initString(xml_string);
  } else {
    fprintf(stderr, "Could not open file [%s] for parsing.\n", filename.c_str());
    return false;
  }
}

/*
bool Model::initParam(const std::string & param)
{
  return initParamWithNodeHandle(param, ros::NodeHandle());
}

bool Model::initParamWithNodeHandle(const std::string & param, const ros::NodeHandle & nh)
{
  std::string xml_string;

  // gets the location of the robot description on the parameter server
  std::string full_param;
  if (!nh.searchParam(param, full_param)){
    ROS_ERROR("Could not find parameter %s on parameter server", param.c_str());
    return false;
  }

  // read the robot description from the parameter server
  if (!nh.getParam(full_param, xml_string)){
    ROS_ERROR("Could not read parameter %s on parameter server", full_param.c_str());
    return false;
  }
  return Model::initString(xml_string);
}
*/

bool Model::initXml(TiXmlDocument * xml_doc)
{
  if (!xml_doc) {
    fprintf(stderr, "Could not parse the xml document.\n");
    return false;
  }

  std::stringstream ss;
  ss << *xml_doc;

  return Model::initString(ss.str());
}

bool Model::initXml(TiXmlElement * robot_xml)
{
  if (!robot_xml) {
    fprintf(stderr, "Could not parse the xml element.\n");
    return false;
  }

  std::stringstream ss;
  ss << (*robot_xml);

  return Model::initString(ss.str());
}

bool Model::initString(const std::string & data)
{
  urdf::ModelInterfaceSharedPtr model;

  size_t best_score = std::numeric_limits<size_t>::max();
  pluginlib::UniquePtr<urdf::URDFParser> best_plugin;
  std::string best_plugin_name;

  // Figure out what plugins might handle this format
  for (const std::string & plugin_name : impl_->loader_.getDeclaredClasses()) {
    pluginlib::UniquePtr<urdf::URDFParser> plugin_instance =
      impl_->loader_.createUniqueInstance(plugin_name);
    if (plugin_instance) {
      size_t score = plugin_instance->might_handle(data);
      if (score < best_score) {
        best_score = score;
        best_plugin = std::move(plugin_instance);
        best_plugin_name = plugin_name;
      }
    }
  }

  if (!best_plugin) {
    fprintf(stderr, "No plugin found for given robot description.\n");
    return false;
  }

  model = best_plugin->parse(data);

  // copy data from model into this object
  if (model) {
    this->links_ = model->links_;
    this->joints_ = model->joints_;
    this->materials_ = model->materials_;
    this->name_ = model->name_;
    this->root_link_ = model->root_link_;
    return true;
  }
  fprintf(stderr, "Failed to parse robot description using: %s\n", best_plugin_name.c_str());
  return false;
}
}  // namespace urdf
