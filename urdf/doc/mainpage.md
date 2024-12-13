urdf::Model is a class containing robot model data structure.

Every Robot Description File (URDF) can be described as a list of Links (urdf::Model::links_) and Joints (urdf::Model::joints_).
The connection between links(nodes) and joints(edges) should define a tree (i.e. 1 parent link, 0+ children links).

* Here is an example Robot Description Describing a Parent Link 'P', a Child Link 'C', and a Joint 'J'
```xml
  <joint name="J" type="revolute">
    <dynamics damping="1" friction="0"/>
    <limit lower="0.9" upper="2.1" effort="1000" velocity="1"/>
    <safety_controller soft_lower_limit="0.7" soft_upper_limit="2.1" k_position="1" k_velocity="1" />
    <calibration reference_position="0.7" />
    <mimic joint="J100" offset="0" multiplier="0.7" />

    <!-- origin: origin of the joint in the parent frame -->
    <!-- child link frame is the joint frame -->
    <!-- axis is in the joint frame -->
    <origin xyz="0 0 0" rpy="0 0 0"/>
    <axis xyz="0 1 0"/>
    <parent link="P"/>
    <child link="C"/>
  </joint>

  <link name="C">
    <inertial>
      <mass value="10"/>
      <origin xyz="0 0 0" rpy="0 0 0"/>
      <inertia ixx="1" ixy="0" ixz="0" iyy="1" iyz="0" izz="1"/>
    </inertial>

    <visual>
      <origin xyz="0 0 0" rpy="0 0 0"/>
      <geometry>
        <box size="1 1 1"/>
      </geometry>
      <material name="Green"/>
    </visual>

    <collision>
      <origin xyz="0 0 0" rpy="0 0 0"/>
      <geometry>
        <box size="1.01 1.01 1.01"/>
      </geometry>
      <contact_coefficient mu="0"  resitution="0"  k_p="0"  k_d="0" />
    </collision>
  </link>

  <material name="Green">
    <texture filename="...texture file..." />
    <!--color rgb="255 255 255" /-->
  </material>
```

# codeapi Code API

The URDF parser API contains the following methods:
  * Parse and build tree from XML: urdf::Model::initXml
  * Parse and build tree from File: urdf::Model::initFile
  * Parse and build tree from String: urdf::Model::initString
  * Get Root Link: urdf::Model::getRoot
  * Get Link by name urdf::Model::getLink
  * Get all Link's urdf::Model::getLinks
  * Get Joint by name urdf::Model::getJoint
