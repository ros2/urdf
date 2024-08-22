// Copyright (c) 2008, Willow Garage, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the name of the copyright holder nor the names of its
//      contributors may be used to endorse or promote products derived from
//      this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

/* Author: Wim Meeussen */

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "urdf/model.hpp"

class TestParser : public testing::TestWithParam<std::vector<std::string>>
{
public:
  bool checkModel(urdf::Model & robot)
  {
    // get root link
    urdf::LinkConstSharedPtr root_link = robot.getRoot();
    if (!root_link) {
      std::cerr << "no root link " << robot.getName() << std::endl;
      return false;
    }

    // go through entire tree
    return this->traverse_tree(root_link);
  }

protected:
  /// constructor
  // num_links starts at 1 because traverse_tree doesn't count the root node
  TestParser()
  : num_joints(0), num_links(1)
  {
  }

  /// Destructor
  ~TestParser()
  {
  }

  bool traverse_tree(urdf::LinkConstSharedPtr link, int level = 0)
  {
    std::cerr << "Traversing tree at level " << level << " link size "
              << link->child_links.size() << std::endl;
    level += 2;
    bool retval = true;
    for (std::vector<urdf::LinkSharedPtr>::const_iterator child = link->child_links.begin();
      child != link->child_links.end(); child++)
    {
      ++num_links;
      if (*child && (*child)->parent_joint) {
        ++num_joints;
        // check rpy
        double roll, pitch, yaw;
        (*child)->parent_joint->parent_to_joint_origin_transform.rotation.getRPY(roll, pitch, yaw);

        if (std::isnan(roll) || std::isnan(pitch) || std::isnan(yaw)) {
          std::cerr << "getRPY() returned nan!" << std::endl;
          return false;
        }
        // recurse down the tree
        retval &= this->traverse_tree(*child, level);
      } else {
        std::cerr << "root link: " << link->name << "has a null child!" << std::endl;
        return false;
      }
    }
    // no more children
    return retval;
  }

  size_t num_joints;
  size_t num_links;
};

TEST_P(TestParser, test) {
  std::vector<std::string> const & input = GetParam();

  std::string folder = _TEST_RESOURCES_DIR_PATH;
  std::cerr << "Folder " << folder << std::endl;
  std::string file = std::string(input[0]);
  bool expect_success = (file.substr(0, 5) != "fail_");
  urdf::Model robot;
  std::cerr << "Parsing file " << (folder + file) << ", expecting " << expect_success << std::endl;
  if (!expect_success) {
    ASSERT_FALSE(robot.initFile(folder + file));
    return;
  }

  std::string robot_name = std::string(input[1]);
  std::string root_name = std::string(input[2]);
  size_t expected_num_joints = atoi(input[3].c_str());
  size_t expected_num_links = atoi(input[4].c_str());

  ASSERT_TRUE(robot.initFile(folder + file));

  EXPECT_EQ(robot.getName(), robot_name);
  urdf::LinkConstSharedPtr root = robot.getRoot();
  ASSERT_TRUE(static_cast<bool>(root));
  EXPECT_EQ(root->name, root_name);

  ASSERT_TRUE(checkModel(robot));
  EXPECT_EQ(num_joints, expected_num_joints);
  EXPECT_EQ(num_links, expected_num_links);
  EXPECT_EQ(robot.joints_.size(), expected_num_joints);
  EXPECT_EQ(robot.links_.size(), expected_num_links);
}

INSTANTIATE_TEST_SUITE_P(GroupTestParser, TestParser, ::testing::Values(
  std::vector<std::string>({"test_robot.urdf", "r2d2", "dummy_link", "16", "17"}),
  std::vector<std::string>({"no_visual.urdf", "no_visual", "link1", "0", "1"}),
  std::vector<std::string>({"one_link.urdf", "one_link", "link1", "0", "1"}),
  std::vector<std::string>({"two_links_one_joint.urdf", "two_links_one_joint", "link1", "1", "2"}),
  // Cases expected not to parse correctly only get filename information (path, urdf file)
  std::vector<std::string>({"fail_pr2_desc_bracket.urdf"}),
  std::vector<std::string>({"fail_three_links_one_joint.urdf"}),
  std::vector<std::string>({"fail_pr2_desc_double_joint.urdf"}),
  std::vector<std::string>({"fail_pr2_desc_loop.urdf"}),
  std::vector<std::string>({"fail_pr2_desc_no_filename_in_mesh.urdf"}),
  std::vector<std::string>({"fail_pr2_desc_no_joint2.urdf"}),
  std::vector<std::string>({"fail_pr2_desc_two_trees.urdf"})
));

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
